#include "BleNus.h"
#include "components/console/Console.h"

using namespace Pinetime::Controllers;

constexpr ble_uuid128_t BleNus::nusServiceUuid;
constexpr ble_uuid128_t BleNus::rxCharacteristicUuid;
constexpr ble_uuid128_t BleNus::txCharacteristicUuid;
uint16_t BleNus::attr_read_handle;
//static uint16_t conn_handle;

int BleNusCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt, void* arg) {
  auto deviceInformationService = static_cast<BleNus*>(arg);
  return deviceInformationService->OnDeviceInfoRequested(conn_handle, attr_handle, ctxt);
}

void BleNus::Init() {
  int res = 0;
  res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);

  res = ble_gatts_add_svcs(serviceDefinition);
  ASSERT(res == 0);
}

void BleNus::SetConnectionHandle(uint16_t connection_handle) {
    conn_handle = connection_handle;
    
}

void BleNus::Print(char *str)
{
  struct os_mbuf *om;
  om = ble_hs_mbuf_from_flat(str, strlen(str));

  if (om) {
      ble_gattc_notify_custom(conn_handle, attr_read_handle, om);
  }
}

void BleNus::ConsoleRegister(std::function<void(char*, int)> f)
{
  this->rxDataFunction = f;
}

int BleNus::OnDeviceInfoRequested(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt) {

    struct os_mbuf *om = ctxt->om;

    switch (ctxt->op) {
        case BLE_GATT_ACCESS_OP_WRITE_CHR:
              while(om) {

                  // reply the received data
                  // test it with Bluefruit, NRF Connect on computer or phone, or in web browser
                  // https://terminal.hardwario.com/

                  rxDataFunction((char *)om->om_data, (int)om->om_len);

                  om = SLIST_NEXT(om, om_next);
              }
              return 0;
        default:
            assert(0);
            return BLE_ATT_ERR_UNLIKELY;
    }
}

BleNus::BleNus()
  : characteristicDefinition {{
                                .uuid = &rxCharacteristicUuid.u,
                                .access_cb = BleNusCallback,
                                .arg = this,
                                .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_WRITE_NO_RSP,
                              },
                              {
                                .uuid = &txCharacteristicUuid.u,
                                .access_cb = BleNusCallback,
                                .arg = this,
                                .flags = BLE_GATT_CHR_F_NOTIFY,
                                .val_handle = &attr_read_handle
                              },
                              {0}},
    serviceDefinition {
      {/* Device Information Service */
       .type = BLE_GATT_SVC_TYPE_PRIMARY,
       .uuid = &nusServiceUuid.u,
       .characteristics = characteristicDefinition},
      {0},
    } {
}
