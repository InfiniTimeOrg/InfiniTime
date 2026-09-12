#include <nrf_log.h>
#include "FSService.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/settings/Settings.h"
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
#ifndef PINETIME_IS_RECOVERY
  if (systemTask.GetSettings().GetDfuAndFsMode() == Pinetime::Controllers::Settings::DfuAndFsMode::Disabled) {
    Pinetime::Controllers::NotificationManager::Notification notif;
    memcpy(notif.message.data(), denyAlert, denyAlertLength);
    notif.size = denyAlertLength;
    notif.category = Pinetime::Controllers::NotificationManager::Categories::SimpleAlert;
    systemTask.GetNotificationManager().Push(std::move(notif));
    systemTask.PushMessage(Pinetime::System::Messages::OnNewNotification);
    return BLE_ATT_ERR_INSUFFICIENT_AUTHOR;
  }
#endif

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

namespace {
  // The path follows its header in the same packet. Both the length field and
  // the buffer it is copied into have to be respected, so check against the
  // bytes that actually arrived and against the destination size.
  bool CopyPath(const char* pathstr, uint16_t pathlen, size_t pathOffset, size_t packetLen, char* out, size_t outSize) {
    if (static_cast<size_t>(pathlen) + 1 > outSize) {
      return false;
    }
    if (pathOffset + static_cast<size_t>(pathlen) > packetLen) {
      return false;
    }
    memcpy(out, pathstr, pathlen);
    out[pathlen] = '\0';
    return true;
  }
}

int FSService::FSCommandHandler(uint16_t connectionHandle, os_mbuf* om) {
  // Every case below reads its header straight out of the received buffer.
  // om_len is how much is really there, so header and path lengths are
  // measured against it before they are used.
  const size_t packetLen = om->om_len;
  if (packetLen < 1) {
    return 0;
  }
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
      if (packetLen < sizeof(ReadHeader)) {
        return 0;
      }
      auto* header = (ReadHeader*) om->om_data;
      if (!CopyPath(header->pathstr, header->pathlen, sizeof(ReadHeader), packetLen, filepath, sizeof(filepath))) {
        return 0;
      }
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
        resp.chunklen = std::min<uint32_t>(std::min<uint32_t>(header->chunksize, info.size), maxChunkLen);
        resp.totallen = info.size;
        fs.FileOpen(&f, filepath, LFS_O_RDONLY);
        fs.FileSeek(&f, header->chunkoff);
        uint8_t fileData[maxChunkLen] = {0};
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
      if (packetLen < sizeof(ReadHeader)) {
        return 0;
      }
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
        resp.chunklen = std::min<uint32_t>(std::min<uint32_t>(header->chunksize, info.size), maxChunkLen);
        resp.totallen = info.size;
        fs.FileOpen(&f, filepath, LFS_O_RDONLY);
        fs.FileSeek(&f, header->chunkoff);
      }
      os_mbuf* om;
      if (resp.chunklen > 0) {
        uint8_t fileData[maxChunkLen] = {0};
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
      if (packetLen < sizeof(WriteHeader)) {
        return 0;
      }
      auto* header = (WriteHeader*) om->om_data;
      if (!CopyPath(header->pathstr, header->pathlen, sizeof(WriteHeader), packetLen, filepath, sizeof(filepath))) {
        return 0; // TODO make this actually return a BLE notif
      }
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
      if (packetLen < sizeof(WritePacing)) {
        return 0;
      }
      auto* header = (WritePacing*) om->om_data;
      // dataSize is announced by the peer. Only what arrived may be written,
      // otherwise the write runs off the end of the packet and into the file.
      const uint32_t dataSize = std::min<uint32_t>(header->dataSize, packetLen - sizeof(WritePacing));
      WriteResponse resp;
      resp.command = commands::WRITE_PACING;
      resp.offset = header->offset;
      int res = 0;

      if (!(res = fs.FileOpen(&f, filepath, LFS_O_RDWR | LFS_O_CREAT))) {
        if ((res = fs.FileSeek(&f, header->offset)) >= 0) {
          res = fs.FileWrite(&f, header->data, dataSize);
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
      if (packetLen < sizeof(DelHeader)) {
        return 0;
      }
      auto* header = (DelHeader*) om->om_data;
      char path[maxpathlen];
      if (!CopyPath(header->pathstr, header->pathlen, sizeof(DelHeader), packetLen, path, sizeof(path))) {
        return 0;
      }
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
      if (packetLen < sizeof(MKDirHeader)) {
        return 0;
      }
      auto* header = (MKDirHeader*) om->om_data;
      char path[maxpathlen];
      if (!CopyPath(header->pathstr, header->pathlen, sizeof(MKDirHeader), packetLen, path, sizeof(path))) {
        return 0;
      }
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
      if (packetLen < sizeof(ListDirHeader)) {
        return 0;
      }
      ListDirHeader* header = (ListDirHeader*) om->om_data;
      char path[maxpathlen];
      if (!CopyPath(header->pathstr, header->pathlen, sizeof(ListDirHeader), packetLen, path, sizeof(path))) {
        return 0;
      }

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
      if (packetLen < sizeof(MoveHeader)) {
        return 0;
      }
      MoveHeader* header = (MoveHeader*) om->om_data;
      // Both paths sit behind the header, separated by a terminator. The old
      // one used to be terminated in place, which wrote into the received
      // packet at an offset the peer chose.
      const size_t oldPathLen = header->OldPathLength;
      char oldPath[maxpathlen];
      char newPath[maxpathlen];
      if (!CopyPath(header->pathstr, header->OldPathLength, sizeof(MoveHeader), packetLen, oldPath, sizeof(oldPath))) {
        return 0;
      }
      if (!CopyPath(&header->pathstr[oldPathLen + 1],
                    header->NewPathLength,
                    sizeof(MoveHeader) + oldPathLen + 1,
                    packetLen,
                    newPath,
                    sizeof(newPath))) {
        return 0;
      }
      MoveResponse resp {};
      resp.command = commands::MOVE_STATUS;
      int8_t res = (int8_t) fs.Rename(oldPath, newPath);
      resp.status = (res == 0) ? 1 : res;
      auto* om = ble_hs_mbuf_from_flat(&resp, sizeof(MoveResponse));
      ble_gattc_notify_custom(connectionHandle, transferCharacteristicHandle, om);
      break;
    }
    default:
      break;
  }
  NRF_LOG_INFO("[FS_S] -> done ");
  systemTask.PushMessage(Pinetime::System::Messages::StopFileTransfer);
  return 0;
}

// Loads resp with file data given a valid filepath header and resp
