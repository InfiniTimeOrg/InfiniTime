#include "components/ble/BatteryInformationService.h"
#include <nrf_log.h>
#include "components/battery/BatteryController.h"

using namespace Pinetime::Controllers;

constexpr ble_uuid16_t BatteryInformationService::batteryInformationServiceUuid;
constexpr ble_uuid16_t BatteryInformationService::batteryLevelUuid;
constexpr ble_uuid16_t BatteryInformationService::batteryLevelStatusUuid; // Renamed

int BatteryInformationServiceCallback(uint16_t /*conn_handle*/, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt, void* arg) {
  auto* batteryInformationService = static_cast<BatteryInformationService*>(arg);
  return batteryInformationService->OnBatteryServiceRequested(attr_handle, ctxt);
}

BatteryInformationService::BatteryInformationService(Controllers::Battery& batteryController)
  : batteryController {batteryController},
    characteristicDefinition {{
      .uuid = &batteryLevelUuid.u,
      .access_cb = BatteryInformationServiceCallback,
      .arg = this,
      .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
      .val_handle = &batteryLevelHandle
    },
    {
      .uuid = &batteryLevelStatusUuid.u, // Updated UUID
      .access_cb = BatteryInformationServiceCallback,
      .arg = this,
      .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
      .val_handle = &batteryLevelStatusHandle // Updated handle
    },
    {0}},
    serviceDefinition {{
      /* Device Information Service */
      .type = BLE_GATT_SVC_TYPE_PRIMARY,
      .uuid = &batteryInformationServiceUuid.u,
      .characteristics = characteristicDefinition
    },
    {0}} {
}

void BatteryInformationService::Init() {
  int res = 0;
  res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);

  res = ble_gatts_add_svcs(serviceDefinition);
  ASSERT(res == 0);
}

int BatteryInformationService::OnBatteryServiceRequested(uint16_t attributeHandle, ble_gatt_access_ctxt* context) {
  if (attributeHandle == batteryLevelHandle) {
    NRF_LOG_INFO("BATTERY : handle = %d", batteryLevelHandle);
    uint8_t batteryValue = batteryController.PercentRemaining();
    int res = os_mbuf_append(context->om, &batteryValue, 1);
    return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

  } else if (attributeHandle == batteryLevelStatusHandle) {
    NRF_LOG_INFO("BATTERY : handle = %d (batteryLevelStatus)", batteryLevelStatusHandle);
    uint8_t powerState = 0;

    powerState |= 1 << 0;
    powerState |= (2 << 1);
    powerState |= (2 << 3);

    if (batteryController.IsCharging()) {
      powerState |= (1 << 5);
    } else {
      powerState &= ~(1 << 5);
    }

    int res = os_mbuf_append(context->om, &powerState, 1);
    return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
  }

  return 0;
}

void BatteryInformationService::NotifyBatteryLevel(uint16_t connectionHandle, uint8_t level) {
  auto* om = ble_hs_mbuf_from_flat(&level, 1);
  ble_gattc_notify_custom(connectionHandle, batteryLevelHandle, om);
}

void BatteryInformationService::NotifyBatteryLevelStatus(uint16_t connectionHandle, bool isCharging) { // Renamed method
  uint8_t powerState = 0;

  powerState |= 1 << 0;
  powerState |= (2 << 1);
  powerState |= (2 << 3);

  if (isCharging) {
    powerState |= (1 << 5);
  } else {
    powerState &= ~(1 << 5);
  }

  auto* om = ble_hs_mbuf_from_flat(&powerState, 1);
  ble_gattc_notify_custom(connectionHandle, batteryLevelStatusHandle, om); // Updated handle
}
