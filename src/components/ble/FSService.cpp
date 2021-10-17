#include <nrf_log.h>
#include "FSService.h"
#include "components/ble/BleController.h"

using namespace Pinetime::Controllers;

constexpr ble_uuid128_t FSService::fsServiceUuid;
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
    case commands::LISTDIR: {
      NRF_LOG_INFO("[FS_S] -> ListDir");
      ListDirHeader *header = (ListDirHeader *)&om->om_data[0];
      uint16_t plen = header->pathlen;
      char path[plen+1] = {0};
      memcpy(path, header->pathstr, plen);
      NRF_LOG_INFO("[FS_S] -> DIR %.10s", path);
      lfs_dir_t dir = {};
      struct lfs_info info = {};

      ListDirResponse resp = {};
      resp.command = 0x51; // LISTDIR_ENTRY;
      resp.status = 1;     // TODO actually use res above!
      resp.totalentries = 0;
      resp.entry = 0;

      int res = fs.DirOpen(path, &dir);
      
      NRF_LOG_INFO("[FS_S] ->diropen %d ", res);
      while (fs.DirRead(&dir, &info)) {
        resp.totalentries++;
        
      }
      NRF_LOG_INFO("[FS_S] -> %d ", resp.totalentries);
      
      fs.DirRewind(&dir);
      
      while (fs.DirRead(&dir, &info)) {
        switch(info.type){
          case LFS_TYPE_REG:
          {
            resp.flags = 0;
            resp.file_size = info.size;
            break;
          } 
          case LFS_TYPE_DIR:
          {
             resp.flags = 1; 
             resp.file_size = 0;
             break;
          }
        }
        resp.modification_time = 0; // TODO Does LFS actually support TS?
        strcpy(resp.path,info.name);
        resp.path_length = strlen(info.name);
        NRF_LOG_INFO("[FS_S] ->Path %s ,", info.name);
        auto* om = ble_hs_mbuf_from_flat(&resp,sizeof(ListDirResponse));
        ble_gattc_notify_custom(connectionHandle,transferCharacteristicHandle,om);
        vTaskDelay(1);  //Allow stuff to actually go out over the BLE conn
        resp.entry++;
      }
      fs.DirClose(&dir);
      resp.file_size = 0;
      resp.path_length = 0;
      resp.flags = 0;
      //Todo this better
      auto* om = ble_hs_mbuf_from_flat(&resp,sizeof(ListDirResponse)-70+resp.path_length);
      ble_gattc_notify_custom(connectionHandle,transferCharacteristicHandle,om);
      NRF_LOG_INFO("[FS_S] -> done ");
      break;
    }
  }
  return 0;
}
