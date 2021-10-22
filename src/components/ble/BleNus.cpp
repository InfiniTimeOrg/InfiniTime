#include "BleNus.h"
#include "components/console/Console.h"

using namespace Pinetime::Controllers;

constexpr ble_uuid128_t BleNus::nusServiceUuid;
constexpr ble_uuid128_t BleNus::rxCharacteristicUuid;
constexpr ble_uuid128_t BleNus::txCharacteristicUuid;
uint16_t BleNus::attributeReadHandle;

int BleNusCallback(uint16_t connectionHandle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt, void* arg) {
  auto deviceInformationService = static_cast<BleNus*>(arg);
  return deviceInformationService->OnDeviceInfoRequested(connectionHandle, attr_handle, ctxt);
}

void BleNus::Init() {
  int res = 0;
  res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);

  res = ble_gatts_add_svcs(serviceDefinition);
  ASSERT(res == 0);
}

void BleNus::SetConnectionHandle(uint16_t connection_handle) {
  connectionHandle = connection_handle;
}

void BleNus::Print(const std::string str) {
  os_mbuf* om;
  om = ble_hs_mbuf_from_flat(str.c_str(), str.length());

  if (om) {
    ble_gattc_notify_custom(connectionHandle, attributeReadHandle, om);
  }
}

void BleNus::RegisterRxCallback(std::function<void(char*, int)> f) {
  this->rxDataFunction = f;
}

int BleNus::OnDeviceInfoRequested(uint16_t connectionHandle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt) {

  os_mbuf* om = ctxt->om;

  switch (ctxt->op) {
    case BLE_GATT_ACCESS_OP_WRITE_CHR:
      while (om) {

        // Test BLE console it with Bluefruit, NRF Toolbox (you must add enter before hitting send!
        // https://devzone.nordicsemi.com/f/nordic-q-a/33687/nrf-toolbox-2-6-0-uart-does-not-send-lf-cr-or-cr-lf-as-eol) on the phone, or in
        // any Chromium-based web browser https://terminal.hardwario.com/

        rxDataFunction((char*) om->om_data, (int) om->om_len);

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
                              {.uuid = &txCharacteristicUuid.u,
                               .access_cb = BleNusCallback,
                               .arg = this,
                               .flags = BLE_GATT_CHR_F_NOTIFY,
                               .val_handle = &attributeReadHandle},
                              {0}},
    serviceDefinition {
      {/* Device Information Service */
       .type = BLE_GATT_SVC_TYPE_PRIMARY,
       .uuid = &nusServiceUuid.u,
       .characteristics = characteristicDefinition},
      {0},
    } {
}
