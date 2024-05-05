#include "components/ble/DfuService.h"
#include <cstring>
#include "components/ble/BleController.h"
#include "drivers/SpiNorFlash.h"
#include "systemtask/SystemTask.h"
#include <nrf_log.h>

using namespace Pinetime::Controllers;

constexpr ble_uuid128_t DfuService::serviceUuid;
constexpr ble_uuid128_t DfuService::controlPointCharacteristicUuid;
constexpr ble_uuid128_t DfuService::revisionCharacteristicUuid;
constexpr ble_uuid128_t DfuService::packetCharacteristicUuid;

int DfuServiceCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt, void* arg) {
  auto dfuService = static_cast<DfuService*>(arg);
  return dfuService->OnServiceData(conn_handle, attr_handle, ctxt);
}

void NotificationTimerCallback(TimerHandle_t xTimer) {
  auto notificationManager = static_cast<DfuService::NotificationManager*>(pvTimerGetTimerID(xTimer));
  notificationManager->OnNotificationTimer();
}

void TimeoutTimerCallback(TimerHandle_t xTimer) {
  auto dfuService = static_cast<DfuService*>(pvTimerGetTimerID(xTimer));
  dfuService->OnTimeout();
}

DfuService::DfuService(Pinetime::System::SystemTask& systemTask,
                       Pinetime::Controllers::Ble& bleController,
                       Pinetime::Drivers::SpiNorFlash& spiNorFlash)
  : systemTask {systemTask},
    bleController {bleController},
    dfuImage {spiNorFlash},
    characteristicDefinition {{
                                .uuid = &packetCharacteristicUuid.u,
                                .access_cb = DfuServiceCallback,
                                .arg = this,
                                .flags = BLE_GATT_CHR_F_WRITE_NO_RSP,
                                .val_handle = nullptr,
                              },
                              {
                                .uuid = &controlPointCharacteristicUuid.u,
                                .access_cb = DfuServiceCallback,
                                .arg = this,
                                .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_NOTIFY,
                                .val_handle = nullptr,
                              },
                              {
                                .uuid = &revisionCharacteristicUuid.u,
                                .access_cb = DfuServiceCallback,
                                .arg = this,
                                .flags = BLE_GATT_CHR_F_READ,
                                .val_handle = &revision,

                              },
                              {0}

    },
    serviceDefinition {
      {/* Device Information Service */
       .type = BLE_GATT_SVC_TYPE_PRIMARY,
       .uuid = &serviceUuid.u,
       .characteristics = characteristicDefinition},
      {0},
    } {
  timeoutTimer = xTimerCreate("notificationTimer", 10000, pdFALSE, this, TimeoutTimerCallback);
}

void DfuService::Init() {
  int res;
  res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);

  res = ble_gatts_add_svcs(serviceDefinition);
  ASSERT(res == 0);
}

int DfuService::OnServiceData(uint16_t connectionHandle, uint16_t attributeHandle, ble_gatt_access_ctxt* context) {
  if (bleController.IsFirmwareUpdating()) {
    xTimerStart(timeoutTimer, 0);
  }

  ble_gatts_find_chr(&serviceUuid.u, &packetCharacteristicUuid.u, nullptr, &packetCharacteristicHandle);
  ble_gatts_find_chr(&serviceUuid.u, &controlPointCharacteristicUuid.u, nullptr, &controlPointCharacteristicHandle);
  ble_gatts_find_chr(&serviceUuid.u, &revisionCharacteristicUuid.u, nullptr, &revisionCharacteristicHandle);

  if (attributeHandle == packetCharacteristicHandle) {
    if (context->op == BLE_GATT_ACCESS_OP_WRITE_CHR)
      return WritePacketHandler(connectionHandle, context->om);
    else
      return 0;
  } else if (attributeHandle == controlPointCharacteristicHandle) {
    if (context->op == BLE_GATT_ACCESS_OP_WRITE_CHR)
      return ControlPointHandler(connectionHandle, context->om);
    else
      return 0;
  } else if (attributeHandle == revisionCharacteristicHandle) {
    if (context->op == BLE_GATT_ACCESS_OP_READ_CHR)
      return SendDfuRevision(context->om);
    else
      return 0;
  } else {
    NRF_LOG_INFO("[DFU] Unknown Characteristic : %d", attributeHandle);
    return 0;
  }
}

int DfuService::SendDfuRevision(os_mbuf* om) const {
  int res = os_mbuf_append(om, &revision, sizeof(revision));
  return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

int DfuService::WritePacketHandler(uint16_t connectionHandle, os_mbuf* om) {
  switch (state) {
    case States::Start: {
      softdeviceSize = om->om_data[0] + (om->om_data[1] << 8) + (om->om_data[2] << 16) + (om->om_data[3] << 24);
      bootloaderSize = om->om_data[4] + (om->om_data[5] << 8) + (om->om_data[6] << 16) + (om->om_data[7] << 24);
      applicationSize = om->om_data[8] + (om->om_data[9] << 8) + (om->om_data[10] << 16) + (om->om_data[11] << 24);
      bleController.FirmwareUpdateTotalBytes(applicationSize);
      NRF_LOG_INFO("[DFU] -> Start data received : SD size : %d, BT size : %d, app size : %d",
                   softdeviceSize,
                   bootloaderSize,
                   applicationSize);

      // wait until SystemTask has finished waking up all devices
      while (systemTask.IsSleeping()) {
        vTaskDelay(50); // 50ms
      }

      dfuImage.Erase();

      uint8_t data[] {16, 1, 1};
      notificationManager.Send(connectionHandle, controlPointCharacteristicHandle, data, 3);
      state = States::Init;
    }
      return 0;
    case States::Init: {
      uint16_t deviceType = om->om_data[0] + (om->om_data[1] << 8);
      uint16_t deviceRevision = om->om_data[2] + (om->om_data[3] << 8);
      uint32_t applicationVersion = om->om_data[4] + (om->om_data[5] << 8) + (om->om_data[6] << 16) + (om->om_data[7] << 24);
      uint16_t softdeviceArrayLength = om->om_data[8] + (om->om_data[9] << 8);
      uint16_t sd[softdeviceArrayLength];
      for (int i = 0; i < softdeviceArrayLength; i++) {
        sd[i] = om->om_data[10 + (i * 2)] + (om->om_data[10 + (i * 2) + 1] << 8);
      }
      expectedCrc = om->om_data[10 + (softdeviceArrayLength * 2)] + (om->om_data[10 + (softdeviceArrayLength * 2) + 1] << 8);

      NRF_LOG_INFO(
        "[DFU] -> Init data received : deviceType = %d, deviceRevision = %d, applicationVersion = %d, nb SD = %d, First SD = %d, CRC = %u",
        deviceType,
        deviceRevision,
        applicationVersion,
        softdeviceArrayLength,
        sd[0],
        expectedCrc);

      return 0;
    }

    case States::Data: {
      nbPacketReceived++;
      dfuImage.Append(om->om_data, om->om_len);
      bytesReceived += om->om_len;
      bleController.FirmwareUpdateCurrentBytes(bytesReceived);

      if ((nbPacketReceived % nbPacketsToNotify) == 0 && bytesReceived != applicationSize) {
        uint8_t data[5] {static_cast<uint8_t>(Opcodes::PacketReceiptNotification),
                         static_cast<uint8_t>(bytesReceived & 0x000000FFu),
                         static_cast<uint8_t>(bytesReceived >> 8u),
                         static_cast<uint8_t>(bytesReceived >> 16u),
                         static_cast<uint8_t>(bytesReceived >> 24u)};
        NRF_LOG_INFO("[DFU] -> Send packet notification: %d bytes received", bytesReceived);
        notificationManager.Send(connectionHandle, controlPointCharacteristicHandle, data, 5);
      }
      if (dfuImage.IsComplete()) {
        uint8_t data[3] {static_cast<uint8_t>(Opcodes::Response),
                         static_cast<uint8_t>(Opcodes::ReceiveFirmwareImage),
                         static_cast<uint8_t>(ErrorCodes::NoError)};
        NRF_LOG_INFO("[DFU] -> Send packet notification : all bytes received!");
        notificationManager.Send(connectionHandle, controlPointCharacteristicHandle, data, 3);
        state = States::Validate;
      }
    }
      return 0;
    default:
      // Invalid state
      return 0;
  }
  return 0;
}

int DfuService::ControlPointHandler(uint16_t connectionHandle, os_mbuf* om) {
  auto opcode = static_cast<Opcodes>(om->om_data[0]);
  NRF_LOG_INFO("[DFU] -> ControlPointHandler");

  switch (opcode) {
    case Opcodes::StartDFU: {
      if (state != States::Idle && state != States::Start) {
        NRF_LOG_INFO("[DFU] -> Start DFU requested, but we are not in Idle state");
        return 0;
      }
      if (state == States::Start) {
        NRF_LOG_INFO("[DFU] -> Start DFU requested, but we are already in Start state");
        return 0;
      }
      auto imageType = static_cast<ImageTypes>(om->om_data[1]);
      if (imageType == ImageTypes::Application) {
        NRF_LOG_INFO("[DFU] -> Start DFU, mode = Application");
        state = States::Start;
        bleController.StartFirmwareUpdate();
        bleController.State(Pinetime::Controllers::Ble::FirmwareUpdateStates::Running);
        bleController.FirmwareUpdateTotalBytes(0xffffffffu);
        bleController.FirmwareUpdateCurrentBytes(0);
        systemTask.PushMessage(Pinetime::System::Messages::BleFirmwareUpdateStarted);
        return 0;
      } else {
        NRF_LOG_INFO("[DFU] -> Start DFU, mode %d not supported!", imageType);
        return 0;
      }
    } break;
    case Opcodes::InitDFUParameters: {
      if (state != States::Init) {
        NRF_LOG_INFO("[DFU] -> Init DFU requested, but we are not in Init state");
        return 0;
      }
      bool isInitComplete = (om->om_data[1] != 0);
      NRF_LOG_INFO("[DFU] -> Init DFU parameters %s", isInitComplete ? " complete" : " not complete");

      if (isInitComplete) {
        uint8_t data[3] {static_cast<uint8_t>(Opcodes::Response),
                         static_cast<uint8_t>(Opcodes::InitDFUParameters),
                         (isInitComplete ? uint8_t {1} : uint8_t {0})};
        notificationManager.AsyncSend(connectionHandle, controlPointCharacteristicHandle, data, 3);
        return 0;
      }
    }
      return 0;
    case Opcodes::PacketReceiptNotificationRequest:
      nbPacketsToNotify = om->om_data[1];
      NRF_LOG_INFO("[DFU] -> Receive Packet Notification Request, nb packet = %d", nbPacketsToNotify);
      return 0;
    case Opcodes::ReceiveFirmwareImage:
      if (state != States::Init) {
        NRF_LOG_INFO("[DFU] -> Receive firmware image requested, but we are not in Start Init");
        return 0;
      }
      // TODO the chunk size is dependent of the implementation of the host application...
      dfuImage.Init(20, applicationSize, expectedCrc);
      NRF_LOG_INFO("[DFU] -> Starting receive firmware");
      state = States::Data;
      return 0;
    case Opcodes::ValidateFirmware: {
      if (state != States::Validate) {
        NRF_LOG_INFO("[DFU] -> Validate firmware image requested, but we are not in Data state %d", state);
        return 0;
      }

      NRF_LOG_INFO("[DFU] -> Validate firmware image requested -- %d", connectionHandle);

      if (dfuImage.Validate()) {
        state = States::Validated;
        bleController.State(Pinetime::Controllers::Ble::FirmwareUpdateStates::Validated);
        NRF_LOG_INFO("Image OK");

        uint8_t data[3] {static_cast<uint8_t>(Opcodes::Response),
                         static_cast<uint8_t>(Opcodes::ValidateFirmware),
                         static_cast<uint8_t>(ErrorCodes::NoError)};
        notificationManager.AsyncSend(connectionHandle, controlPointCharacteristicHandle, data, 3);
      } else {
        NRF_LOG_INFO("Image Error : bad CRC");

        uint8_t data[3] {static_cast<uint8_t>(Opcodes::Response),
                         static_cast<uint8_t>(Opcodes::ValidateFirmware),
                         static_cast<uint8_t>(ErrorCodes::CrcError)};
        notificationManager.AsyncSend(connectionHandle, controlPointCharacteristicHandle, data, 3);
        bleController.State(Pinetime::Controllers::Ble::FirmwareUpdateStates::Error);
        Reset();
      }

      return 0;
    }
    case Opcodes::ActivateImageAndReset:
      if (state != States::Validated) {
        NRF_LOG_INFO("[DFU] -> Activate image and reset requested, but we are not in Validated state");
        return 0;
      }
      NRF_LOG_INFO("[DFU] -> Activate image and reset!");
      bleController.State(Pinetime::Controllers::Ble::FirmwareUpdateStates::Validated);
      Reset();
      return 0;
    default:
      return 0;
  }
}

void DfuService::OnTimeout() {
  bleController.State(Pinetime::Controllers::Ble::FirmwareUpdateStates::Error);
  Reset();
}

void DfuService::Reset() {
  state = States::Idle;
  nbPacketsToNotify = 0;
  nbPacketReceived = 0;
  bytesReceived = 0;
  softdeviceSize = 0;
  bootloaderSize = 0;
  applicationSize = 0;
  expectedCrc = 0;
  notificationManager.Reset();
  bleController.StopFirmwareUpdate();
  systemTask.PushMessage(Pinetime::System::Messages::BleFirmwareUpdateFinished);
}

DfuService::NotificationManager::NotificationManager() {
  timer = xTimerCreate("notificationTimer", 1000, pdFALSE, this, NotificationTimerCallback);
}

bool DfuService::NotificationManager::AsyncSend(uint16_t connection, uint16_t charactHandle, uint8_t* data, size_t s) {
  if (size != 0 || s > 10)
    return false;

  connectionHandle = connection;
  characteristicHandle = charactHandle;
  size = s;
  std::memcpy(buffer, data, size);
  xTimerStart(timer, 0);
  return true;
}

void DfuService::NotificationManager::OnNotificationTimer() {
  if (size > 0) {
    Send(connectionHandle, characteristicHandle, buffer, size);
    size = 0;
  }
}

void DfuService::NotificationManager::Send(uint16_t connection, uint16_t charactHandle, const uint8_t* data, const size_t s) {
  auto* om = ble_hs_mbuf_from_flat(data, s);
  auto ret = ble_gattc_notify_custom(connection, charactHandle, om);
  ASSERT(ret == 0);
}

void DfuService::NotificationManager::Reset() {
  connectionHandle = 0;
  characteristicHandle = 0;
  size = 0;
  xTimerStop(timer, 0);
}

void DfuService::DfuImage::Init(size_t chunkSize, size_t totalSize, uint16_t expectedCrc) {
  if (chunkSize != 20)
    return;
  this->chunkSize = chunkSize;
  this->totalSize = totalSize;
  this->expectedCrc = expectedCrc;
  this->ready = true;
  totalWriteIndex = 0;
  bufferWriteIndex = 0;
}

void DfuService::DfuImage::Append(uint8_t* data, size_t size) {
  if (!ready)
    return;
  ASSERT(size <= 20);

  std::memcpy(tempBuffer + bufferWriteIndex, data, size);
  bufferWriteIndex += size;

  if (bufferWriteIndex == bufferSize) {
    spiNorFlash.Write(writeOffset + totalWriteIndex, tempBuffer, bufferWriteIndex);
    totalWriteIndex += bufferWriteIndex;
    bufferWriteIndex = 0;
  }

  if (bufferWriteIndex > 0 && totalWriteIndex + bufferWriteIndex == totalSize) {
    spiNorFlash.Write(writeOffset + totalWriteIndex, tempBuffer, bufferWriteIndex);
    totalWriteIndex += bufferWriteIndex;
    if (totalSize < maxSize)
      WriteMagicNumber();
  }
}

void DfuService::DfuImage::WriteMagicNumber() {
  uint32_t magic[4] = {
    // TODO When this variable is a static constexpr, the values written to the memory are not correct. Why?
    0xf395c277,
    0x7fefd260,
    0x0f505235,
    0x8079b62c,
  };

  uint32_t offset = writeOffset + (maxSize - (4 * sizeof(uint32_t)));
  spiNorFlash.Write(offset, reinterpret_cast<const uint8_t*>(magic), 4 * sizeof(uint32_t));
}

void DfuService::DfuImage::Erase() {
  for (size_t erased = 0; erased < maxSize; erased += 0x1000) {
    spiNorFlash.SectorErase(writeOffset + erased);
  }
}

bool DfuService::DfuImage::Validate() {
  uint32_t chunkSize = 200;
  size_t currentOffset = 0;
  uint16_t crc = 0;

  bool first = true;
  while (currentOffset < totalSize) {
    uint32_t readSize = (totalSize - currentOffset) > chunkSize ? chunkSize : (totalSize - currentOffset);

    spiNorFlash.Read(writeOffset + currentOffset, tempBuffer, readSize);
    if (first) {
      crc = ComputeCrc(tempBuffer, readSize, NULL);
      first = false;
    } else
      crc = ComputeCrc(tempBuffer, readSize, &crc);
    currentOffset += readSize;
  }

  return (crc == expectedCrc);
}

uint16_t DfuService::DfuImage::ComputeCrc(uint8_t const* p_data, uint32_t size, uint16_t const* p_crc) {
  uint16_t crc = (p_crc == NULL) ? 0xFFFF : *p_crc;

  for (uint32_t i = 0; i < size; i++) {
    crc = static_cast<uint8_t>(crc >> 8) | (crc << 8);
    crc ^= p_data[i];
    crc ^= static_cast<uint8_t>(crc & 0xFF) >> 4;
    crc ^= (crc << 8) << 4;
    crc ^= ((crc & 0xFF) << 4) << 1;
  }

  return crc;
}

bool DfuService::DfuImage::IsComplete() {
  if (!ready)
    return false;
  return totalWriteIndex == totalSize;
}
