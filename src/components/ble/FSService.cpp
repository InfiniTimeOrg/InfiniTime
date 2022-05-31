#include <nrf_log.h>
#include "FSService.h"
#include "components/ble/BleController.h"
#include "systemtask/SystemTask.h"

using namespace Pinetime::Controllers;

constexpr ble_uuid16_t FSService::fsServiceUuid;
constexpr ble_uuid128_t FSService::fsVersionUuid;
constexpr ble_uuid128_t FSService::fsTransferUuid;

int FSServiceCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt, void* arg) {
  auto* fsService = static_cast<FSService*>(arg);
  return fsService->OnFSServiceRequested(conn_handle, attr_handle, ctxt);
}

FSService::FSService(Pinetime::System::SystemTask& systemTask, Pinetime::Controllers::FS& fs)
  : systemTask {systemTask},
    fs {fs},
    characteristicDefinition {{.uuid = &fsVersionUuid.u,
                               .access_cb = FSServiceCallback,
                               .arg = this,
                               .flags = BLE_GATT_CHR_F_READ,
                               .val_handle = &versionCharacteristicHandle},
                              {
                                .uuid = &fsTransferUuid.u,
                                .access_cb = FSServiceCallback,
                                .arg = this,
                                .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                                .val_handle = &transferCharacteristicHandle,
                              },
                              {0}},
    serviceDefinition {
      {/* Device Information Service */
       .type = BLE_GATT_SVC_TYPE_PRIMARY,
       .uuid = &fsServiceUuid.u,
       .characteristics = characteristicDefinition},
      {0},
    } {
}

void FSService::Init() {
  int res = 0;
  res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);

  res = ble_gatts_add_svcs(serviceDefinition);
  ASSERT(res == 0);
}

int FSService::OnFSServiceRequested(uint16_t connectionHandle, uint16_t attributeHandle, ble_gatt_access_ctxt* context) {
  if (attributeHandle == versionCharacteristicHandle) {
    NRF_LOG_INFO("FS_S : handle = %d", versionCharacteristicHandle);
    int res = os_mbuf_append(context->om, &fsVersion, sizeof(fsVersion));
    return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
  }
  if (attributeHandle == transferCharacteristicHandle) {
    return FSCommandHandler(connectionHandle, context->om);
  }
  return 0;
}

int FSService::FSCommandHandler(uint16_t connectionHandle, os_mbuf* om) {
  auto command = static_cast<commands>(om->om_data[0]);
  NRF_LOG_INFO("[FS_S] -> FSCommandHandler Command %d", command);
  // Just always make sure we are awake...
  systemTask.PushMessage(Pinetime::System::Messages::StartFileTransfer);
  vTaskDelay(10);
  while (systemTask.IsSleeping()) {
    vTaskDelay(100); // 50ms
  }
  lfs_dir_t dir = {0};
  lfs_info info = {0};
  lfs_file f = {0};
  switch (command) {
    case commands::READ: {
      NRF_LOG_INFO("[FS_S] -> Read");
      auto* header = (ReadHeader*) om->om_data;
      uint16_t plen = header->pathlen;
      if (plen > maxpathlen) { //> counts for null term
        return -1;
      }
      memcpy(filepath, header->pathstr, plen);
      filepath[plen] = 0; // Copy and null terminate string
      ReadResponse resp;
      os_mbuf* om;
      resp.command = commands::READ_DATA;
      resp.status = 0x01;
      resp.chunkoff = header->chunkoff;
      int res = fs.Stat(filepath, &info);
      if (res == LFS_ERR_NOENT && info.type != LFS_TYPE_DIR) {
        resp.status = (int8_t) res;
        resp.chunklen = 0;
        resp.totallen = 0;
        om = ble_hs_mbuf_from_flat(&resp, sizeof(ReadResponse));
      } else {
        resp.chunklen = std::min(header->chunksize, info.size); // TODO add mtu somehow
        resp.totallen = info.size;
        fs.FileOpen(&f, filepath, LFS_O_RDONLY);
        fs.FileSeek(&f, header->chunkoff);
        uint8_t fileData[resp.chunklen] = {0};
        resp.chunklen = fs.FileRead(&f, fileData, resp.chunklen);
        om = ble_hs_mbuf_from_flat(&resp, sizeof(ReadResponse));
        os_mbuf_append(om, fileData, resp.chunklen);
        fs.FileClose(&f);
      }

      ble_gattc_notify_custom(connectionHandle, transferCharacteristicHandle, om);
      break;
    }
    case commands::READ_PACING: {
      NRF_LOG_INFO("[FS_S] -> Readpacing");
      auto* header = (ReadHeader*) om->om_data;
      ReadResponse resp;
      resp.command = commands::READ_DATA;
      resp.status = 0x01;
      resp.chunkoff = header->chunkoff;
      int res = fs.Stat(filepath, &info);
      if (res == LFS_ERR_NOENT && info.type != LFS_TYPE_DIR) {
        resp.status = (int8_t) res;
        resp.chunklen = 0;
        resp.totallen = 0;
      } else {
        resp.chunklen = std::min(header->chunksize, info.size); // TODO add mtu somehow
        resp.totallen = info.size;
        fs.FileOpen(&f, filepath, LFS_O_RDONLY);
        fs.FileSeek(&f, header->chunkoff);
      }
      os_mbuf* om;
      if (resp.chunklen > 0) {
        uint8_t fileData[resp.chunklen] = {0};
        resp.chunklen = fs.FileRead(&f, fileData, resp.chunklen);
        om = ble_hs_mbuf_from_flat(&resp, sizeof(ReadResponse));
        os_mbuf_append(om, fileData, resp.chunklen);
      } else {
        resp.chunklen = 0;
        om = ble_hs_mbuf_from_flat(&resp, sizeof(ReadResponse));
      }
      fs.FileClose(&f);
      ble_gattc_notify_custom(connectionHandle, transferCharacteristicHandle, om);
      break;
    }
    case commands::WRITE: {
      NRF_LOG_INFO("[FS_S] -> Write");
      auto* header = (WriteHeader*) om->om_data;
      uint16_t plen = header->pathlen;
      if (plen > maxpathlen) { //> counts for null term
        return -1;             // TODO make this actually return a BLE notif
      }
      memcpy(filepath, header->pathstr, plen);
      filepath[plen] = 0; // Copy and null terminate string
      fileSize = header->totalSize;
      WriteResponse resp;
      resp.command = commands::WRITE_PACING;
      resp.offset = header->offset;
      resp.modTime = 0;

      int res = fs.FileOpen(&f, filepath, LFS_O_RDWR | LFS_O_CREAT);
      if (res == 0) {
        fs.FileClose(&f);
        resp.status = (res == 0) ? 0x01 : (int8_t) res;
      }
      resp.freespace = std::min(fs.getSize() - (fs.GetFSSize() * fs.getBlockSize()), fileSize - header->offset);
      auto* om = ble_hs_mbuf_from_flat(&resp, sizeof(WriteResponse));
      ble_gattc_notify_custom(connectionHandle, transferCharacteristicHandle, om);
      break;
    }
    case commands::WRITE_DATA: {
      NRF_LOG_INFO("[FS_S] -> WriteData");
      auto* header = (WritePacing*) om->om_data;
      WriteResponse resp;
      resp.command = commands::WRITE_PACING;
      resp.offset = header->offset;
      int res = 0;

      if (!(res = fs.FileOpen(&f, filepath, LFS_O_RDWR | LFS_O_CREAT))) {
        if ((res = fs.FileSeek(&f, header->offset)) >= 0) {
          res = fs.FileWrite(&f, header->data, header->dataSize);
        }
        fs.FileClose(&f);
      }
      if (res < 0) {
        resp.status = (int8_t) res;
      }
      resp.freespace = std::min(fs.getSize() - (fs.GetFSSize() * fs.getBlockSize()), fileSize - header->offset);
      auto* om = ble_hs_mbuf_from_flat(&resp, sizeof(WriteResponse));
      ble_gattc_notify_custom(connectionHandle, transferCharacteristicHandle, om);
      break;
    }
    case commands::DELETE: {
      NRF_LOG_INFO("[FS_S] -> Delete");
      auto* header = (DelHeader*) om->om_data;
      uint16_t plen = header->pathlen;
      char path[plen + 1] = {0};
      memcpy(path, header->pathstr, plen);
      path[plen] = 0; // Copy and null terminate string
      DelResponse resp {};
      resp.command = commands::DELETE_STATUS;
      int res = fs.FileDelete(path);
      resp.status = (res == 0) ? 0x01 : (int8_t) res;
      auto* om = ble_hs_mbuf_from_flat(&resp, sizeof(DelResponse));
      ble_gattc_notify_custom(connectionHandle, transferCharacteristicHandle, om);
      break;
    }
    case commands::MKDIR: {
      NRF_LOG_INFO("[FS_S] -> MKDir");
      auto* header = (MKDirHeader*) om->om_data;
      uint16_t plen = header->pathlen;
      char path[plen + 1] = {0};
      memcpy(path, header->pathstr, plen);
      path[plen] = 0; // Copy and null terminate string
      MKDirResponse resp {};
      resp.command = commands::MKDIR_STATUS;
      resp.modification_time = 0;
      int res = fs.DirCreate(path);
      resp.status = (res == 0) ? 0x01 : (int8_t) res;
      auto* om = ble_hs_mbuf_from_flat(&resp, sizeof(MKDirResponse));
      ble_gattc_notify_custom(connectionHandle, transferCharacteristicHandle, om);
      break;
    }
    case commands::LISTDIR: {
      NRF_LOG_INFO("[FS_S] -> ListDir");
      ListDirHeader* header = (ListDirHeader*) om->om_data;
      uint16_t plen = header->pathlen;
      char path[plen + 1] = {0};
      path[plen] = 0; // Copy and null terminate string
      memcpy(path, header->pathstr, plen);

      ListDirResponse resp {};

      resp.command = commands::LISTDIR_ENTRY;
      resp.status = 0x01;
      resp.totalentries = 0;
      resp.entry = 0;
      resp.modification_time = 0;
      int res = fs.DirOpen(path, &dir);
      if (res != 0) {
        resp.status = (int8_t) res;
        auto* om = ble_hs_mbuf_from_flat(&resp, sizeof(ListDirResponse));
        ble_gattc_notify_custom(connectionHandle, transferCharacteristicHandle, om);
        break;
      };
      while (fs.DirRead(&dir, &info)) {
        resp.totalentries++;
      }
      fs.DirRewind(&dir);
      while (true) {
        res = fs.DirRead(&dir, &info);
        if (res <= 0) {
          break;
        }
        switch (info.type) {
          case LFS_TYPE_REG: {
            resp.flags = 0;
            resp.file_size = info.size;
            break;
          }
          case LFS_TYPE_DIR: {
            resp.flags = 1;
            resp.file_size = 0;
            break;
          }
        }

        // strcpy(resp.path, info.name);
        resp.path_length = strlen(info.name);
        auto* om = ble_hs_mbuf_from_flat(&resp, sizeof(ListDirResponse));
        os_mbuf_append(om, info.name, resp.path_length);
        ble_gattc_notify_custom(connectionHandle, transferCharacteristicHandle, om);
        /*
         * Todo Figure out how to know when the previous Notify was TX'd
         * For now just delay 100ms to make sure that the data went out...
         */
        vTaskDelay(100); // Allow stuff to actually go out over the BLE conn
        resp.entry++;
      }
      assert(fs.DirClose(&dir) == 0);
      resp.file_size = 0;
      resp.path_length = 0;
      resp.flags = 0;
      auto* om = ble_hs_mbuf_from_flat(&resp, sizeof(ListDirResponse));
      ble_gattc_notify_custom(connectionHandle, transferCharacteristicHandle, om);
      break;
    }
    case commands::MOVE: {
      NRF_LOG_INFO("[FS_S] -> Move");
      MoveHeader* header = (MoveHeader*) om->om_data;
      uint16_t plen = header->OldPathLength;
      // Null Terminate string
      header->pathstr[plen] = 0;
      char path[header->NewPathLength + 1] = {0};
      memcpy(path, &header->pathstr[plen + 1], header->NewPathLength);
      path[header->NewPathLength] = 0; // Copy and null terminate string
      MoveResponse resp {};
      resp.command = commands::MOVE_STATUS;
      int8_t res = (int8_t) fs.Rename(header->pathstr, path);
      resp.status = (res == 0) ? 1 : res;
      auto* om = ble_hs_mbuf_from_flat(&resp, sizeof(MoveResponse));
      ble_gattc_notify_custom(connectionHandle, transferCharacteristicHandle, om);
    }
    default:
      break;
  }
  NRF_LOG_INFO("[FS_S] -> done ");
  systemTask.PushMessage(Pinetime::System::Messages::StopFileTransfer);
  return 0;
}

// Loads resp with file data given a valid filepath header and resp
void FSService::prepareReadDataResp(ReadHeader* header, ReadResponse* resp) {
  // uint16_t plen = header->pathlen;
  resp->command = commands::READ_DATA;
  resp->chunkoff = header->chunkoff;
  resp->status = 0x01;
  struct lfs_info info = {};
  int res = fs.Stat(filepath, &info);
  if (res == LFS_ERR_NOENT && info.type != LFS_TYPE_DIR) {
    resp->status = 0x03;
    resp->chunklen = 0;
    resp->totallen = 0;
  } else {
    lfs_file f;
    resp->chunklen = std::min(header->chunksize, info.size);
    resp->totallen = info.size;
    fs.FileOpen(&f, filepath, LFS_O_RDONLY);
    fs.FileSeek(&f, header->chunkoff);
    resp->chunklen = fs.FileRead(&f, resp->chunk, resp->chunklen);
    fs.FileClose(&f);
  }
}
