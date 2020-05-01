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
    NRF_LOG_INFO("[DFU] %s Packet", op);
    if(context->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
//      NRF_LOG_INFO("[DFU] -> Write  %dB", context->om->om_len);
      if(opcode == 1) {
        uint8_t data[3]{16, opcode, param};
        NRF_LOG_INFO("[DFU] -> Send notification: {%d, %d, %d}", data[0], data[1], data[2]);

        auto *om = ble_hs_mbuf_from_flat(data, 3);
        ble_gattc_notify_custom(connectionHandle, controlPointCharacteristicHandle, om);
      }
      if(dataMode){
        nbPacketReceived++;
        bytesReceived += context->om->om_len;
        NRF_LOG_INFO("[DFU] -> Bytes received : %d in %d packets", bytesReceived, nbPacketReceived);

        if((nbPacketReceived % nbPacketsToNotify) == 0) {
          uint8_t data[5]{17, (uint8_t)(bytesReceived>>24),(uint8_t)(bytesReceived>>16), (uint8_t)(bytesReceived>>8), (uint8_t)(bytesReceived&0x000000FF) };
          NRF_LOG_INFO("[DFU] -> Send packet notification: %d bytes received",bytesReceived);

          auto *om = ble_hs_mbuf_from_flat(data, 5);
          ble_gattc_notify_custom(connectionHandle, controlPointCharacteristicHandle, om);
        }
        if(bytesReceived == 175280) {
          uint8_t data[3]{16, 3, 1};
          NRF_LOG_INFO("[DFU] -> Send packet notification : all bytes received!");

          auto *om = ble_hs_mbuf_from_flat(data, 3);
          ble_gattc_notify_custom(connectionHandle, controlPointCharacteristicHandle, om);
        }
      }

    }
  } else if (attributeHandle == controlPointCharacteristicHandle) {
    NRF_LOG_INFO("[DFU] %s ControlPoint", op);
    if(context->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
//      NRF_LOG_INFO("[DFU] -> Write  %dB {%d, %d}", context->om->om_len, context->om->om_data[0], context->om->om_data[1]);
      switch(context->om->om_data[0]) {
        case 0x01: {// START DFU
          NRF_LOG_INFO("[DFU] -> Start DFU, mode = %d", context->om->om_data[1]);
          opcode = 0x01;
          param = 1;
        }
          break;
        case 0x02:
          NRF_LOG_INFO("[DFU] -> Receive init, state (0=RX, 1=Complete) = %d", context->om->om_data[1]);
          opcode = 0x02;
          param = context->om->om_data[1];
          if(param == 1) {
            uint8_t data[3] {16, opcode, param};
            NRF_LOG_INFO("[DFU] -> Send notification: {%d, %d, %d}", data[0], data[1], data[2]);

            auto *om = ble_hs_mbuf_from_flat(data, 3);

            ble_gattc_notify_custom(connectionHandle, controlPointCharacteristicHandle, om);
          }
          break;
        case 0x08:
          nbPacketsToNotify = context->om->om_data[1];
          NRF_LOG_INFO("[DFU] -> Receive Packet Notification Request, nb packet = %d", nbPacketsToNotify);
          break;
        case 0x03:
          NRF_LOG_INFO("[DFU] -> Starting receive firmware");
          dataMode = true;
          break;
        case 0x04: {
          NRF_LOG_INFO("[DFU] -> Validate firmware");
          uint8_t data[3]{16, 4, 1};
          NRF_LOG_INFO("[DFU] -> Send notification: {%d, %d, %d}", data[0], data[1], data[2]);

          auto *om = ble_hs_mbuf_from_flat(data, 3);

          ble_gattc_notify_custom(connectionHandle, controlPointCharacteristicHandle, om);
        }
          break;
        case 0x05:
          NRF_LOG_INFO("[DFU] -> Activate image and reset!");
          break;
      }


    }

  } else if(attributeHandle == revisionCharacteristicHandle) {
    NRF_LOG_INFO("[DFU] %s Revision", op);
    if(context->op == BLE_GATT_ACCESS_OP_READ_CHR) {
      int res = os_mbuf_append(context->om, &revision, 2);
      return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    }
  } else {
      NRF_LOG_INFO("[DFU] Unknown Characteristic : %d - %s", attributeHandle, op);
  }

  return 0;
}
