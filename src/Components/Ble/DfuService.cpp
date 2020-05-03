#include <Components/Ble/BleController.h>
#include <SystemTask/SystemTask.h>
#include "DfuService.h"

using namespace Pinetime::Controllers;

constexpr ble_uuid128_t DfuService::serviceUuid;
constexpr ble_uuid128_t DfuService::controlPointCharacteristicUuid;
constexpr ble_uuid128_t DfuService::revisionCharacteristicUuid;
constexpr ble_uuid128_t DfuService::packetCharacteristicUuid;

int DfuServiceCallback(uint16_t conn_handle, uint16_t attr_handle,
                       struct ble_gatt_access_ctxt *ctxt, void *arg) {
  auto dfuService = static_cast<DfuService*>(arg);
  return dfuService->OnServiceData(conn_handle, attr_handle, ctxt);
}

DfuService::DfuService(Pinetime::System::SystemTask& systemTask, Pinetime::Controllers::Ble& bleController) :
        systemTask{systemTask},
        bleController{bleController},
        characteristicDefinition{
                {
                        .uuid = (ble_uuid_t *) &packetCharacteristicUuid,
                        .access_cb = DfuServiceCallback,
                        .arg = this,
                        .flags = BLE_GATT_CHR_F_WRITE_NO_RSP,
                        .val_handle = nullptr,
                },
                {
                        .uuid = (ble_uuid_t *) &controlPointCharacteristicUuid,
                        .access_cb = DfuServiceCallback,
                        .arg = this,
                        .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_NOTIFY,
                        .val_handle = nullptr,
                },
                {
                        .uuid = (ble_uuid_t *) &revisionCharacteristicUuid,
                        .access_cb = DfuServiceCallback,
                        .arg = this,
                        .flags = BLE_GATT_CHR_F_READ,
                        .val_handle = &revision,

                },
                {
                  0
                }

        },
        serviceDefinition {
                {
                        /* Device Information Service */
                        .type = BLE_GATT_SVC_TYPE_PRIMARY,
                        .uuid = (ble_uuid_t *) &serviceUuid,
                        .characteristics = characteristicDefinition
                },
                {
                        0
                },
        }

        {

}

void DfuService::Init() {
  ble_gatts_count_cfg(serviceDefinition);
  ble_gatts_add_svcs(serviceDefinition);
}

int DfuService::OnServiceData(uint16_t connectionHandle, uint16_t attributeHandle, ble_gatt_access_ctxt *context) {

  ble_gatts_find_chr((ble_uuid_t*)&serviceUuid, (ble_uuid_t*)&packetCharacteristicUuid, nullptr, &packetCharacteristicHandle);
  ble_gatts_find_chr((ble_uuid_t*)&serviceUuid, (ble_uuid_t*)&controlPointCharacteristicUuid, nullptr, &controlPointCharacteristicHandle);
  ble_gatts_find_chr((ble_uuid_t*)&serviceUuid, (ble_uuid_t*)&revisionCharacteristicUuid, nullptr, &revisionCharacteristicHandle);

  if(attributeHandle == packetCharacteristicHandle) {
    if(context->op == BLE_GATT_ACCESS_OP_WRITE_CHR)
      return WritePacketHandler(connectionHandle, context->om);
    else return 0;
  } else if(attributeHandle == controlPointCharacteristicHandle) {
    if(context->op == BLE_GATT_ACCESS_OP_WRITE_CHR)
        return ControlPointHandler(connectionHandle, context->om);
    else return 0;
  } else if(attributeHandle == revisionCharacteristicHandle) {
    if(context->op == BLE_GATT_ACCESS_OP_READ_CHR)
      return SendDfuRevision(context->om);
    else return 0;
  } else {
    NRF_LOG_INFO("[DFU] Unknown Characteristic : %d", attributeHandle);
    return 0;
  }
}

int DfuService::SendDfuRevision(os_mbuf *om) const {
  int res = os_mbuf_append(om, &revision, sizeof(revision));
  return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

int DfuService::WritePacketHandler(uint16_t connectionHandle, os_mbuf *om) {
  switch(state) {
    case States::Start: {
      softdeviceSize = om->om_data[0] + (om->om_data[1] << 8) + (om->om_data[2] << 16) + (om->om_data[3] << 24);
      bootloaderSize = om->om_data[4] + (om->om_data[5] << 8) + (om->om_data[6] << 16) + (om->om_data[7] << 24);
      applicationSize = om->om_data[8] + (om->om_data[9] << 8) + (om->om_data[10] << 16) + (om->om_data[11] << 24);
      NRF_LOG_INFO("[DFU] -> Start data received : SD size : %d, BT size : %d, app size : %d", softdeviceSize, bootloaderSize, applicationSize);

      uint8_t data[] {16, 1, 1};
      SendNotification(connectionHandle, data, 3);
      state = States::Init;
    }
      return 0;
    case States::Init: {
      uint16_t deviceType = om->om_data[0] + (om->om_data[1] << 8);
      uint16_t deviceRevision = om->om_data[2] + (om->om_data[3] << 8);
      uint32_t applicationVersion = om->om_data[4] + (om->om_data[5] << 8) + (om->om_data[6] << 16) + (om->om_data[7] << 24);
      uint16_t softdeviceArrayLength = om->om_data[8] + (om->om_data[9] << 8);
      uint16_t sd[softdeviceArrayLength];
      for(int i = 0; i < softdeviceArrayLength; i++) {
        sd[i] = om->om_data[10 + (i*2)] + (om->om_data[(i*2)+1] << 8);
      }
      uint16_t crc = om->om_data[10 + (softdeviceArrayLength*2)] + (om->om_data[10 + (softdeviceArrayLength*2)] << 8);

      NRF_LOG_INFO("[DFU] -> Init data received : deviceType = %d, deviceRevision = %d, applicationVersion = %d, nb SD = %d, First SD = %d, CRC = %d",
                   deviceType, deviceRevision, applicationVersion, softdeviceArrayLength, sd[0], crc);

      return 0;
    }

    case States::Data: {
      nbPacketReceived++;
      bytesReceived += om->om_len;
      bleController.FirmwareUpdateCurrentBytes(bytesReceived);
      NRF_LOG_INFO("[DFU] -> Bytes received : %d in %d packets", bytesReceived, nbPacketReceived);

      if((nbPacketReceived % nbPacketsToNotify) == 0) {
        uint8_t data[5]{static_cast<uint8_t>(Opcodes::PacketReceiptNotification),
                        (uint8_t)(bytesReceived&0x000000FFu),(uint8_t)(bytesReceived>>8u), (uint8_t)(bytesReceived>>16u),(uint8_t)(bytesReceived>>24u) };
        NRF_LOG_INFO("[DFU] -> Send packet notification: %d bytes received",bytesReceived);
        SendNotification(connectionHandle, data, 5);
      }
      if(bytesReceived == applicationSize) {
        uint8_t data[3]{static_cast<uint8_t>(Opcodes::Response),
                        static_cast<uint8_t>(Opcodes::ReceiveFirmwareImage),
                        static_cast<uint8_t>(ErrorCodes::NoError)};
        NRF_LOG_INFO("[DFU] -> Send packet notification : all bytes received!");
        SendNotification(connectionHandle, data, 3);
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

int DfuService::ControlPointHandler(uint16_t connectionHandle, os_mbuf *om) {
  auto opcode = static_cast<Opcodes>(om->om_data[0]);
  NRF_LOG_INFO("[DFU] -> ControlPointHandler");

  switch(opcode) {
    case Opcodes::StartDFU: {
      if(state != States::Idle && state != States::Start) {
        NRF_LOG_INFO("[DFU] -> Start DFU requested, but we are not in Idle state");
        return 0;
      }
      if(state == States::Start) {
        NRF_LOG_INFO("[DFU] -> Start DFU requested, but we are already in Start state");
        return 0;
      }
      auto imageType = static_cast<ImageTypes>(om->om_data[1]);
      if(imageType == ImageTypes::Application) {
        NRF_LOG_INFO("[DFU] -> Start DFU, mode = Application");
        state = States::Start;
        bleController.StartFirmwareUpdate();
        bleController.FirmwareUpdateTotalBytes(175280);
        bleController.FirmwareUpdateCurrentBytes(0);
        systemTask.PushMessage(Pinetime::System::SystemTask::Messages::BleFirmwareUpdateStarted);
        return 0;
      } else {
        NRF_LOG_INFO("[DFU] -> Start DFU, mode %d not supported!", imageType);
        return 0;
      }
    }
      break;
    case Opcodes::InitDFUParameters: {
      if (state != States::Init) {
        NRF_LOG_INFO("[DFU] -> Init DFU requested, but we are not in Init state");
        return 0;
      }
      bool isInitComplete = (om->om_data[1] != 0);
      NRF_LOG_INFO("[DFU] -> Init DFU parameters %s", isInitComplete ? " complete" : " not complete");

      if (isInitComplete) {
        uint8_t data[3]{static_cast<uint8_t>(Opcodes::Response),
                        static_cast<uint8_t>(Opcodes::InitDFUParameters),
                        (isInitComplete ? uint8_t{1} : uint8_t{0})};
        NRF_LOG_INFO("SEND NOTIF : %d %d %d", data[0], data[1], data[2]);
        SendNotification(connectionHandle, data, 3);
        return 0;
      }
    }
      return 0;
    case Opcodes::PacketReceiptNotificationRequest:
      nbPacketsToNotify = om->om_data[1];
      NRF_LOG_INFO("[DFU] -> Receive Packet Notification Request, nb packet = %d", nbPacketsToNotify);
      return 0;
    case Opcodes::ReceiveFirmwareImage:
      if(state != States::Init) {
        NRF_LOG_INFO("[DFU] -> Receive firmware image requested, but we are not in Start Init");
        return 0;
      }
      NRF_LOG_INFO("[DFU] -> Starting receive firmware");
      state = States::Data;
      return 0;
    case Opcodes::ValidateFirmware: {
      if(state != States::Validate) {
        NRF_LOG_INFO("[DFU] -> Validate firmware image requested, but we are not in Data state");
        return 0;
      }
      NRF_LOG_INFO("[DFU] -> Validate firmware");
      state = States::Validated;
      uint8_t data[3]{static_cast<uint8_t>(Opcodes::Response),
                      static_cast<uint8_t>(Opcodes::ValidateFirmware),
                      static_cast<uint8_t>(ErrorCodes::NoError)};
      SendNotification(connectionHandle, data, 3);
      return 0;
    }
    case Opcodes::ActivateImageAndReset:
      if(state != States::Validated) {
        NRF_LOG_INFO("[DFU] -> Activate image and reset requested, but we are not in Validated state");
        return 0;
      }
      NRF_LOG_INFO("[DFU] -> Activate image and reset!");
      bleController.StopFirmwareUpdate();
      systemTask.PushMessage(Pinetime::System::SystemTask::Messages::BleFirmwareUpdateFinished);
      return 0;
    default: return 0;
  }
}

void DfuService::SendNotification(uint16_t connectionHandle, const uint8_t *data, const size_t size) {
  auto *om = ble_hs_mbuf_from_flat(data, size);
  auto ret = ble_gattc_notify_custom(connectionHandle, controlPointCharacteristicHandle, om);
  ASSERT(ret == 0);
}
