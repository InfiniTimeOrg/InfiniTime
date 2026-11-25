#include "components/ble/BatteryInformationService.h"
#include <nrf_log.h>
#include "components/battery/BatteryController.h"

using namespace Pinetime::Controllers;

constexpr ble_uuid16_t BatteryInformationService::batteryInformationServiceUuid;
constexpr ble_uuid16_t BatteryInformationService::batteryLevelUuid;
constexpr ble_uuid16_t BatteryInformationService::batteryLevelStatusUuid;

namespace {
  namespace PowerStateBits {
    constexpr uint16_t batteryPresent = 1 << 0;

    constexpr uint16_t externalPowerNo = 0;
    constexpr uint16_t externalPowerYes = 1;
    constexpr uint8_t wiredShift = 1;
    constexpr uint8_t wirelessShift = 3;

    enum class ChargeState : uint16_t { Unknown = 0, Charging = 1, DischargingActive = 2, DischargingInactive = 3 };
    constexpr uint8_t chargeStateShift = 5;
  }

  constexpr uint8_t batteryLevelStatusFlags = 0;
}

int BatteryInformationServiceCallback(uint16_t /*conn_handle*/, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt, void* arg) {
  auto* batteryInformationService = static_cast<BatteryInformationService*>(arg);
  return batteryInformationService->OnBatteryServiceRequested(attr_handle, ctxt);
}

BatteryInformationService::BatteryInformationService(Controllers::Battery& batteryController)
  : batteryController {batteryController},
    characteristicDefinition {{.uuid = &batteryLevelUuid.u,
                               .access_cb = BatteryInformationServiceCallback,
                               .arg = this,
                               .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                               .val_handle = &batteryLevelHandle},
                              {.uuid = &batteryLevelStatusUuid.u,
                               .access_cb = BatteryInformationServiceCallback,
                               .arg = this,
                               .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                               .val_handle = &batteryLevelStatusHandle},
                              {0}},
    serviceDefinition {
      {/* Battery Service */
       .type = BLE_GATT_SVC_TYPE_PRIMARY,
       .uuid = &batteryInformationServiceUuid.u,
       .characteristics = characteristicDefinition},
      {0},
    } {
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
  }
  if (attributeHandle == batteryLevelStatusHandle) {
    const uint16_t powerState = PowerState();
    const uint8_t value[3] = {batteryLevelStatusFlags, static_cast<uint8_t>(powerState & 0xFF), static_cast<uint8_t>(powerState >> 8)};
    int res = os_mbuf_append(context->om, value, sizeof(value));
    return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
  }
  return 0;
}

uint16_t BatteryInformationService::PowerState() const {
  using namespace PowerStateBits;
  const bool powerPresent = batteryController.IsPowerPresent();

  ChargeState chargeState;
  if (batteryController.IsCharging()) {
    chargeState = ChargeState::Charging;
  } else if (powerPresent) {
    chargeState = ChargeState::DischargingInactive;
  } else {
    chargeState = ChargeState::DischargingActive;
  }

  return batteryPresent | ((powerPresent ? externalPowerYes : externalPowerNo) << wiredShift) | (externalPowerNo << wirelessShift) |
         (static_cast<uint16_t>(chargeState) << chargeStateShift);
}

void BatteryInformationService::NotifyBatteryLevel(uint16_t connectionHandle, uint8_t level) {
  auto* om = ble_hs_mbuf_from_flat(&level, 1);
  ble_gattc_notify_custom(connectionHandle, batteryLevelHandle, om);
}

void BatteryInformationService::NotifyBatteryLevelStatus() {
  const uint16_t powerState = PowerState();
  if (powerState == lastPowerState) {
    return;
  }
  lastPowerState = powerState;
  ble_gatts_chr_updated(batteryLevelStatusHandle);
}
