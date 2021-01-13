#include "FtpService.h"

#include <LittleFs.h>

using namespace Pinetime::Controllers;

// provide linktime space for constants
constexpr ble_uuid128_t FtpService::serviceUuid;
constexpr ble_uuid128_t FtpService::controlPointCharacteristicUuid;
constexpr ble_uuid128_t FtpService::revisionCharacteristicUuid;
constexpr ble_uuid128_t FtpService::packetCharacteristicUuid;

/// C level hook to call class methods
int FtpServiceCallback(uint16_t conn_handle, uint16_t attr_handle,
                       struct ble_gatt_access_ctxt *ctxt, void *arg) {
  auto ftpService = static_cast<FtpService *>(arg);
  return ftpService->OnServiceData(conn_handle, attr_handle, ctxt);
}

FtpService::FtpService(Pinetime::System::SystemTask &systemTask,
                       Pinetime::Controllers::Ble &bleController,
                       Pinetime::System::LittleFs &littleFs)
: mSystemTask{systemTask}, mBleController{bleController}, mLittleFs{littleFs},
  mCharacteristicDefinitions{
      { .uuid = reinterpret_cast<const ble_uuid_t*>(&revisionCharacteristicUuid),
        .access_cb = FtpServiceCallback, .arg = this,
        .flags = BLE_GATT_CHR_F_READ, .val_handle = nullptr },
      { .uuid = reinterpret_cast<const ble_uuid_t*>(&pathCharacteristicUuid),
        .access_cb = FtpServiceCallback, .arg = this,
        .flags = (BLE_GATT_CHR_F_WRITE_NO_RSP | BLE_GATT_CHR_F_READ), .val_handle = nullptr },
      { .uuid = reinterpret_cast<const ble_uuid_t*>(&controlPointCharacteristicUuid),
        .access_cb = FtpServiceCallback, .arg = this,
        .flags = (BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_NOTIFY), .val_handle = nullptr },
      { .uuid = reinterpret_cast<const ble_uuid_t*>(&packetCharacteristicUuid),
        .access_cb = FtpServiceCallback, .arg = this,
        .flags = (BLE_GATT_CHR_F_WRITE_NO_RSP | BLE_GATT_CHR_F_READ), .val_handle = nullptr },
      { 0 /* null terminator */ }
  },
  mServiceDefinitions{
      { .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = reinterpret_cast<const ble_uuid_t*>(&serviceUuid),
        .characteristics = mCharacteristicDefinitions
      },
      { 0 /* null terminator */ }
  }
{

}

void FtpService::Init() {
  int res;
  res = ble_gatts_count_cfg(mServiceDefinitions);
  ASSERT(res == 0);

  res = ble_gatts_add_svcs(mServiceDefinitions);
  ASSERT(res == 0);
}

int FtpService::OnServiceData(uint16_t connectionHandle, uint16_t attributeHandle, ble_gatt_access_ctxt *context) {

  { uint16_t revisionCharacteristicHandle;
    ble_gatts_find_chr(reinterpret_cast<const ble_uuid_t*>(&serviceUuid),
                       reinterpret_cast<const ble_uuid_t*>(&revisionCharacteristicUuid),
                       nullptr /* don't get defintion handle */,
                       &revisionCharacteristicHandle);
    if(attributeHandle == revisionCharacteristicHandle) {
      const int rtn = os_mbuf_append(context->om, &revision, sizeof(revision));
      return (rtn == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
  }

  { uint16_t pathCharacteristicHandle;
    ble_gatts_find_chr(reinterpret_cast<const ble_uuid_t*>(&serviceUuid),
                       reinterpret_cast<const ble_uuid_t*>(&pathCharacteristicUuid),
                       nullptr /* don't get defintion handle */,
                       &pathCharacteristicHandle);
    if(attributeHandle == pathCharacteristicHandle) {
      switch(context->op) {
        case BLE_GATT_ACCESS_OP_READ_CHR: {
          const int rtn = os_mbuf_append(context->om, &mCurrentPath, mCurrentPath_sz);
          return (rtn == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        case BLE_GATT_ACCESS_OP_WRITE_CHR: {
          closeActive();
          mCurrentPath_sz = context->om->om_len;
          memcpy(mCurrentPath, context->om->om_data, mCurrentPath_sz);
          // always force in null terminator
          mCurrentPath[mCurrentPath_sz - 1] = '\0';
          return 0;
        }
        default:
          // nimble should never have called us
          NRF_LOG_INFO("[DFU] Unknown Characteristic Access : %d", context->op);
          return BLE_ATT_ERR_UNLIKELY;
      }
    }
  }

  { uint16_t controlPointCharacteristicHandle;
    ble_gatts_find_chr(reinterpret_cast<const ble_uuid_t*>(&serviceUuid),
                       reinterpret_cast<const ble_uuid_t*>(&controlPointCharacteristicUuid),
                       nullptr /* don't get defintion handle */,
                       &controlPointCharacteristicHandle);
    if(attributeHandle == controlPointCharacteristicHandle) {
      const auto opcode = static_cast<Opcodes>(context->om->om_data[0]);
      switch(opcode) {
        case Opcodes::REMOVE: {
          closeActive();
          int rtn = mLittleFs.remove(mCurrentPath);
          uint8_t data[1];
          data[0] = (rtn == 0) ? static_cast<uint8_t>(ResultCodes::SUCCESS) :
                                  static_cast<uint8_t>(ResultCodes::FAILURE);
          rtn = os_mbuf_append(context->om, &data, sizeof(data));
          return (rtn == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        case Opcodes::MKDIR: {
          closeActive();
          int rtn = mLittleFs.mkdir(mCurrentPath);
          uint8_t data[1];
          data[0] = (rtn == 0) ? static_cast<uint8_t>(ResultCodes::SUCCESS) :
                                  static_cast<uint8_t>(ResultCodes::FAILURE);
          rtn = os_mbuf_append(context->om, &data, sizeof(data));
          return (rtn == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        case Opcodes::READ: {
          closeActive();
          mFile = mLittleFs.open_danger(mCurrentPath, LFS_O_RDONLY);
          uint8_t data[1];
          data[0] = ((mFile == nullptr) || mFile->isNULL()) ?
                        static_cast<uint8_t>(ResultCodes::FAILURE) :
                        static_cast<uint8_t>(ResultCodes::SUCCESS);
          const int rtn = os_mbuf_append(context->om, &data, sizeof(data));
          return (rtn == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        case Opcodes::WRITE: {
          closeActive();
          mFile = mLittleFs.open_danger(mCurrentPath, (LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC | LFS_O_APPEND));
          uint8_t data[1];
          data[0] = ((mFile == nullptr) || mFile->isNULL()) ?
                        static_cast<uint8_t>(ResultCodes::FAILURE) :
                        static_cast<uint8_t>(ResultCodes::SUCCESS);
          const int rtn = os_mbuf_append(context->om, &data, sizeof(data));
          return (rtn == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
          break;
        }
        case Opcodes::CLOSE: {
          closeActive();
          const uint8_t data[1] = { static_cast<uint8_t>(ResultCodes::SUCCESS) };
          const int rtn = os_mbuf_append(context->om, &data, sizeof(data));
          return (rtn == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        case Opcodes::LIST: {
          closeActive();
          mDir = mLittleFs.opendir_danger(mCurrentPath);
          uint8_t data[1];
          data[0] = ((mDir == nullptr) || mDir->isNULL()) ?
                        static_cast<uint8_t>(ResultCodes::FAILURE) :
                        static_cast<uint8_t>(ResultCodes::SUCCESS);
          const int rtn = os_mbuf_append(context->om, &data, sizeof(data));
          return (rtn == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        default:
          return BLE_ATT_ERR_REQ_NOT_SUPPORTED;
      }
    }
  }

  { uint16_t packetCharacteristicHandle;
    ble_gatts_find_chr(reinterpret_cast<const ble_uuid_t*>(&serviceUuid),
                      reinterpret_cast<const ble_uuid_t*>(&packetCharacteristicUuid),
                      nullptr /* don't get defintion handle */,
                      &packetCharacteristicHandle);
    if(attributeHandle == packetCharacteristicHandle) {
      switch(context->op) {
        case BLE_GATT_ACCESS_OP_WRITE_CHR: {
          return handleWrite(context);
        }
        case BLE_GATT_ACCESS_OP_READ_CHR: {
          return handleRead(context);
        }
        default:
          // nimble should never have called us
          NRF_LOG_INFO("[DFU] Unknown Characteristic : %d", attributeHandle);
          return BLE_ATT_ERR_UNLIKELY;
      }
    }
  }

  // nimble should never have called us
  NRF_LOG_INFO("[DFU] Unknown Characteristic : %d", attributeHandle);
  return BLE_ATT_ERR_UNLIKELY;
}

void FtpService::closeActive() {
  // deletion will invoke the destructor which releases LFS resources
  delete mFile;
  delete mDir;
}

int FtpService::handleWrite(ble_gatt_access_ctxt* context) {
  return -1;
}

int FtpService::handleRead(ble_gatt_access_ctxt* context) {
  return -1;
}
