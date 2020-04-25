#include <hal/nrf_rtc.h>
#include "CurrentTimeClient.h"

using namespace Pinetime::Controllers;

constexpr ble_uuid16_t CurrentTimeClient::ctsServiceUuid;
constexpr ble_uuid16_t CurrentTimeClient::currentTimeCharacteristicUuid;

int Pinetime::Controllers::CurrentTimeDiscoveryEventCallback(uint16_t conn_handle, const struct ble_gatt_error *error,
                           const struct ble_gatt_svc *service, void *arg) {
  auto client = static_cast<CurrentTimeClient*>(arg);
  return client->OnDiscoveryEvent(conn_handle, error, service);
}

int Pinetime::Controllers::CurrentTimeCharacteristicDiscoveredCallback(uint16_t conn_handle, const struct ble_gatt_error *error,
                                                const struct ble_gatt_chr *chr, void *arg) {
  auto client = static_cast<CurrentTimeClient*>(arg);
  return client->OnCharacteristicDiscoveryEvent(conn_handle, error, chr);
}

int Pinetime::Controllers::CurrentTimeReadCallback(uint16_t conn_handle, const struct ble_gatt_error *error,
                                   struct ble_gatt_attr *attr, void *arg) {
  auto client = static_cast<CurrentTimeClient*>(arg);
  return client->OnCurrentTimeReadResult(conn_handle, error, attr);
}


CurrentTimeClient::CurrentTimeClient(DateTime& dateTimeController) : dateTimeController{dateTimeController} {

}

void CurrentTimeClient::Init() {

}

void CurrentTimeClient::StartDiscovery(uint16_t connectionHandle) {
  ble_gattc_disc_svc_by_uuid(connectionHandle, ((ble_uuid_t*)&ctsServiceUuid), CurrentTimeDiscoveryEventCallback, this);
}

bool CurrentTimeClient::OnDiscoveryEvent(uint16_t connectionHandle, const ble_gatt_error *error, const ble_gatt_svc *service) {
  if(service == nullptr && error->status == BLE_HS_EDONE) {
    NRF_LOG_INFO("CTS Discovery complete");
    return true;
  }

  if(service != nullptr && ble_uuid_cmp(((ble_uuid_t*)&ctsServiceUuid), &service->uuid.u) == 0) {
    NRF_LOG_INFO("CTS discovered : 0x%x",  service->start_handle);
    ble_gattc_disc_chrs_by_uuid(connectionHandle, service->start_handle, service->end_handle, ((ble_uuid_t*)&currentTimeCharacteristicUuid), CurrentTimeCharacteristicDiscoveredCallback, this);
  }
  return false;
}

int CurrentTimeClient::OnCharacteristicDiscoveryEvent(uint16_t conn_handle, const ble_gatt_error *error,
                                   const ble_gatt_chr *characteristic) {
  if(characteristic == nullptr && error->status == BLE_HS_EDONE)
    NRF_LOG_INFO("CTS Characteristic discovery complete");

  if(characteristic != nullptr && ble_uuid_cmp(((ble_uuid_t*)&currentTimeCharacteristicUuid), &characteristic->uuid.u) == 0) {
    NRF_LOG_INFO("CTS Characteristic discovered : 0x%x", characteristic->val_handle);

    ble_gattc_read(conn_handle, characteristic->val_handle, CurrentTimeReadCallback, this);
  }
  return 0;
}

int CurrentTimeClient::OnCurrentTimeReadResult(uint16_t conn_handle, const ble_gatt_error *error, const ble_gatt_attr *attribute) {
  if(error->status == 0) {
    // TODO check that attribute->handle equals the handle discovered in OnCharacteristicDiscoveryEvent
    CtsData result;
    os_mbuf_copydata(attribute->om, 0, sizeof(CtsData), &result);
    NRF_LOG_INFO("Received data: %d-%d-%d %d:%d:%d", result.year,
                 result.month, result.dayofmonth,
                 result.hour, result.minute, result.second);
    dateTimeController.SetTime(result.year, result.month, result.dayofmonth,
                               0, result.hour, result.minute, result.second, nrf_rtc_counter_get(portNRF_RTC_REG));
  } else {
    NRF_LOG_INFO("Error retrieving current time: %d", error->status);
  }
  return 0;
}
