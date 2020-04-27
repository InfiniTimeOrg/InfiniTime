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

DfuService::DfuService() :
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

  /*     *     o  BLE_GATT_ACCESS_OP_READ_CHR
     *     o  BLE_GATT_ACCESS_OP_WRITE_CHR
     *     o  BLE_GATT_ACCESS_OP_READ_DSC
     *     o  BLE_GATT_ACCESS_OP_WRITE_DSC
     *     */

  char* op;
  switch(context->op) {
    case BLE_GATT_ACCESS_OP_READ_CHR: op = "Read Characteristic"; break;
    case BLE_GATT_ACCESS_OP_WRITE_CHR: op = "Write Characteristic"; break;
    case BLE_GATT_ACCESS_OP_READ_DSC: op = "Read Descriptor"; break;
    case BLE_GATT_ACCESS_OP_WRITE_DSC: op = "Write Descriptor"; break;
  }

  if(attributeHandle == packetCharacteristicHandle) {
    NRF_LOG_INFO("[DFU] Packet Characteristic : %d - %s", attributeHandle, op);
    if(context->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
      NRF_LOG_INFO("[DFU] -> Write  %dB", context->om->om_len);
      uint8_t data[3] {16, 1, 1};
      auto* om = ble_hs_mbuf_from_flat(data, 3);
      ble_gattc_notify_custom(connectionHandle, controlPointCharacteristicHandle, om);
    }
  } else if (attributeHandle == controlPointCharacteristicHandle) {
    NRF_LOG_INFO("[DFU] ControlPoint Characteristic : %d - %s", attributeHandle, op);
    if(context->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
      NRF_LOG_INFO("[DFU] -> Write  %dB", context->om->om_len);
      switch(context->om->om_data[0]) {
        case 0x01: {// START DFU
          NRF_LOG_INFO("[DFU] -> Start DFU, mode = %d", context->om->om_data[1]);

        }

          break;
      }

    }

  } else if(attributeHandle == revisionCharacteristicHandle) {
    NRF_LOG_INFO("[DFU] Revision Characteristic : %d - %s", attributeHandle, op);
    if(context->op == BLE_GATT_ACCESS_OP_READ_CHR) {
      int res = os_mbuf_append(context->om, &revision, 2);
      return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
  } else {
      NRF_LOG_INFO("[DFU] Unknown Characteristic : %d - %s", attributeHandle, op);
  }

  return 0;
}
