#include <nrf_log.h>
#include "FSService.h"
#include "components/ble/BleController.h"

using namespace Pinetime::Controllers;

constexpr ble_uuid16_t FSService::fsServiceUuid;
constexpr ble_uuid128_t FSService::fsVersionUuid;
constexpr ble_uuid128_t FSService::fsTransferUuid;

int FSServiceCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt, void* arg) {
  auto* fsService = static_cast<FSService*>(arg);
  return fsService->OnFSServiceRequested(conn_handle, attr_handle, ctxt);
}

FSService::FSService(Pinetime::Controllers::FS& fs)
  : fs {fs},
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
  NRF_LOG_INFO("[FS_S] -> FSCommandHandler");

  switch (command) {
    case commands::READ: {
      NRF_LOG_INFO("[FS_S] -> Read");
      if (state != FSState::IDLE) {
        return -1;
      }
      state = FSState::READ;
      auto* header = (ReadHeader*) om->om_data;
      uint16_t plen = header->pathlen;
      if (plen > maxpathlen - 1) {
        return -1;
      }
      memcpy(filepath, header->pathstr, plen);
      filepath[plen + 1] = 0; // Copy and null teminate string
      ReadResponse resp;
      resp.command = commands::READ_DATA;
      resp.chunkoff = header->chunkoff;
      resp.status = 0x01;
      struct lfs_info info = {};
      int res = fs.Stat(filepath, &info);
      if (res == LFS_ERR_NOENT && info.type != LFS_TYPE_DIR) {
        resp.status = 0x03;
        resp.chunklen = 0;
        resp.totallen = 0;
      } else {
        lfs_file f;
        resp.chunklen = std::min(header->chunksize, info.size);
        resp.totallen = info.size;
        fs.FileOpen(&f, filepath, LFS_O_RDONLY);
        fs.FileSeek(&f, header->chunkoff);
        resp.chunklen = fs.FileRead(&f, resp.chunk, resp.chunklen);
        fs.FileClose(&f);
      }
      auto* om = ble_hs_mbuf_from_flat(&resp, sizeof(ReadResponse));
      ble_gattc_notify_custom(connectionHandle, transferCharacteristicHandle, om);
      if (header->chunksize >= resp.totallen) { // probably removeable...but then usafe
        state = FSState::IDLE;
      }
    }
    case commands::READ_PACING: {
      NRF_LOG_INFO("[FS_S] -> ReadPacing");
      if (state != FSState::READ) {
        return -1;
      }
      auto* header = (ReadPacing*) om->om_data;
      ReadResponse resp = {};

      resp.command = commands::READ_DATA;
      resp.chunkoff = header->chunkoff;
      resp.status = 0x01;
      struct lfs_info info = {};
      int res = fs.Stat(filepath, &info);
      if (res == LFS_ERR_NOENT && info.type != LFS_TYPE_DIR) {
        resp.status = 0x03;
        resp.chunklen = 0;
        resp.totallen = 0;
      } else {
        lfs_file f;
        resp.chunklen = std::min(header->chunksize, info.size);
        resp.totallen = info.size;
        fs.FileOpen(&f, filepath, LFS_O_RDONLY);
        fs.FileSeek(&f, header->chunkoff);
        resp.chunklen = fs.FileRead(&f, resp.chunk, resp.chunklen);
        fs.FileClose(&f);
      }
      auto* om = ble_hs_mbuf_from_flat(&resp, sizeof(ReadResponse));
      ble_gattc_notify_custom(connectionHandle, transferCharacteristicHandle, om);
      if (resp.chunklen >= header->chunksize) { // is this right?
        state = FSState::IDLE;
      }
    }
    case commands::WRITE: {
      if (state != FSState::IDLE) {
        return -1;
      }
    }
    case commands::WRITE_PACING: {
      if (state != FSState::WRITE) {
        return -1;
      }
    }

    case commands::DELETE: {
      NRF_LOG_INFO("[FS_S] -> Delete");
      auto* header = (DelHeader*) om->om_data;
      uint16_t plen = header->pathlen;
      char path[plen + 1] = {0};
      struct lfs_info info = {};
      DelResponse resp = {};
      resp.command = commands::DELETE_STATUS;
      int res = fs.Stat(path, &info);
      // Get Info about path
      // branch for DirDel of FileDelete
      if (info.type == LFS_TYPE_DIR) {
        res = fs.DirDelete(path);
      } else {
        res = fs.FileDelete(path);
      }
      switch (res) {
        case LFS_ERR_OK:
          resp.status = 0x01;
          break;
        default:
          resp.status = 0x02;
          break;
      }
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
      NRF_LOG_INFO("[FS_S] -> MKDIR %.10s", path);
      MKDirResponse resp = {};
      resp.command = commands::MKDIR_STATUS;
      int res = fs.DirCreate(path);
      switch (res) {
        case LFS_ERR_OK:
          resp.status = 0x01;
          break;
        default:
          resp.status = 0x02;
          break;
      }
      resp.modification_time = 0; // We should timestamp..but no
      auto* om = ble_hs_mbuf_from_flat(&resp, sizeof(MKDirResponse));
      ble_gattc_notify_custom(connectionHandle, transferCharacteristicHandle, om);
      break;
    }
    case commands::LISTDIR: {
      NRF_LOG_INFO("[FS_S] -> ListDir");
      ListDirHeader* header = (ListDirHeader*) om->om_data;
      uint16_t plen = header->pathlen;
      char path[plen + 1] = {0};
      memcpy(path, header->pathstr, plen);
      NRF_LOG_INFO("[FS_S] -> DIR %.10s", path);
      lfs_dir_t dir = {};
      struct lfs_info info = {};

      ListDirResponse resp = {};
      resp.command = commands::LISTDIR_ENTRY;
      resp.status = 1; // TODO actually use res above!
      resp.totalentries = 0;
      resp.entry = 0;

      int res = fs.DirOpen(path, &dir);

      NRF_LOG_INFO("[FS_S] ->diropen %d ", res);
      while (fs.DirRead(&dir, &info)) {
        resp.totalentries++;
      }
      NRF_LOG_INFO("[FS_S] -> %d ", resp.totalentries);

      fs.DirRewind(&dir);

      while (true) {
        int res = fs.DirRead(&dir, &info);
        if(res <= 0){
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
        resp.modification_time = 0; // TODO Does LFS actually support TS?
        strcpy(resp.path, info.name);
        resp.path_length = strlen(info.name);
        NRF_LOG_INFO("[FS_S] ->Path %s ,", info.name);
        auto* om = ble_hs_mbuf_from_flat(&resp, sizeof(ListDirResponse)+resp.path_length);
        ble_gattc_notify_custom(connectionHandle, transferCharacteristicHandle, om);
        vTaskDelay(5); // Allow stuff to actually go out over the BLE conn
        resp.entry++;
      }
      fs.DirClose(&dir);
      resp.file_size = 0;
      resp.path_length = 0;
      resp.flags = 0;
      // TODO Handle Size of response better.
      auto* om = ble_hs_mbuf_from_flat(&resp, sizeof(ListDirResponse)+resp.path_length);
      ble_gattc_notify_custom(connectionHandle, transferCharacteristicHandle, om);
      NRF_LOG_INFO("[FS_S] -> done ");
      break;
    }
  }
  return 0;
}
// Loads resp with file data given a valid filepath header and resp
void FSService::prepareReadDataResp(ReadHeader* header, ReadResponse* resp) {
  uint16_t plen = header->pathlen;
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
