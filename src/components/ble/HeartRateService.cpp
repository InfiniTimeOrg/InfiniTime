#include "HeartRateService.h"
#include "components/heartrate/HeartRateController.h"
#include "systemtask/SystemTask.h"

using namespace Pinetime::Controllers;

constexpr ble_uuid16_t HeartRateService::heartRateServiceUuid;
constexpr ble_uuid16_t HeartRateService::heartRateMeasurementUuid;

namespace {
  int HeartRateServiceServiceCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt, void* arg) {
    auto* heartRateService = static_cast<HeartRateService*>(arg);
    return heartRateService->OnHeartRateRequested(conn_handle, attr_handle, ctxt);
  }
}

// TODO Refactoring - remove dependency to SystemTask
HeartRateService::HeartRateService(Pinetime::System::SystemTask& system, Controllers::HeartRateController& heartRateController)
  : system {system},
    heartRateController {heartRateController},
    characteristicDefinition {{.uuid = (ble_uuid_t*) &heartRateMeasurementUuid,
                               .access_cb = HeartRateServiceServiceCallback,
                               .arg = this,
                               .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                               .val_handle = &heartRateMeasurementHandle},
                              {0}},
    serviceDefinition {
      {/* Device Information Service */
       .type = BLE_GATT_SVC_TYPE_PRIMARY,
       .uuid = (ble_uuid_t*) &heartRateServiceUuid,
       .characteristics = characteristicDefinition},
      {0},
    } {
  // TODO refactor to prevent this loop dependency (service depends on controller and controller depends on service)
  heartRateController.SetService(this);
}

void HeartRateService::Init() {
  int res = 0;
  res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);

  res = ble_gatts_add_svcs(serviceDefinition);
  ASSERT(res == 0);
}

int HeartRateService::OnHeartRateRequested(uint16_t connectionHandle, uint16_t attributeHandle, ble_gatt_access_ctxt* context) {
  if (attributeHandle == heartRateMeasurementHandle) {
    NRF_LOG_INFO("HEARTRATE : handle = %d", heartRateMeasurementHandle);
    uint8_t buffer[2] = {0, heartRateController.HeartRate()}; // [0] = flags, [1] = hr value

    int res = os_mbuf_append(context->om, buffer, 2);
    return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
  }
  return 0;
}

void HeartRateService::OnNewHeartRateValue(uint8_t heartRateValue) {
  uint8_t buffer[2] = {0, heartRateController.HeartRate()}; // [0] = flags, [1] = hr value
  auto* om = ble_hs_mbuf_from_flat(buffer, 2);

  uint16_t connectionHandle = system.nimble().connHandle();

  if (connectionHandle == 0 || connectionHandle == BLE_HS_CONN_HANDLE_NONE) {
    return;
  }

  ble_gattc_notify_custom(connectionHandle, heartRateMeasurementHandle, om);
}
