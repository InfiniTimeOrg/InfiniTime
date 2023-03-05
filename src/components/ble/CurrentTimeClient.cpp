#include "components/ble/CurrentTimeClient.h"
#include <nrf_log.h>
#include "components/datetime/DateTimeController.h"

using namespace Pinetime::Controllers;

constexpr ble_uuid16_t CurrentTimeClient::ctsServiceUuid;
constexpr ble_uuid16_t CurrentTimeClient::currentTimeCharacteristicUuid;

namespace {
  int OnDiscoveryEventCallback(uint16_t conn_handle, const struct ble_gatt_error* error, const struct ble_gatt_svc* service, void* arg) {
    auto client = static_cast<CurrentTimeClient*>(arg);
    return client->OnDiscoveryEvent(conn_handle, error, service);
  }

  int OnCurrentTimeCharacteristicDiscoveredCallback(uint16_t conn_handle,
                                                    const struct ble_gatt_error* error,
                                                    const struct ble_gatt_chr* chr,
                                                    void* arg) {
    auto client = static_cast<CurrentTimeClient*>(arg);
    return client->OnCharacteristicDiscoveryEvent(conn_handle, error, chr);
  }

  int CurrentTimeReadCallback(uint16_t conn_handle, const struct ble_gatt_error* error, struct ble_gatt_attr* attr, void* arg) {
    auto client = static_cast<CurrentTimeClient*>(arg);
    return client->OnCurrentTimeReadResult(conn_handle, error, attr);
  }
}

CurrentTimeClient::CurrentTimeClient(DateTime& dateTimeController) : dateTimeController {dateTimeController} {
}

void CurrentTimeClient::Init() {
}

bool CurrentTimeClient::OnDiscoveryEvent(uint16_t connectionHandle, const ble_gatt_error* error, const ble_gatt_svc* service) {
  if (service == nullptr && error->status == BLE_HS_EDONE) {
    if (isDiscovered) {
      NRF_LOG_INFO("CTS found, starting characteristics discovery");

      ble_gattc_disc_all_chrs(connectionHandle, ctsStartHandle, ctsEndHandle, OnCurrentTimeCharacteristicDiscoveredCallback, this);
    } else {
      NRF_LOG_INFO("CTS not found");
      onServiceDiscovered(connectionHandle);
    }
    return true;
  }

  if (service != nullptr && ble_uuid_cmp(&ctsServiceUuid.u, &service->uuid.u) == 0) {
    NRF_LOG_INFO("CTS discovered : 0x%x - 0x%x", service->start_handle, service->end_handle);
    isDiscovered = true;
    ctsStartHandle = service->start_handle;
    ctsEndHandle = service->end_handle;
    return false;
  }
  return false;
}

int CurrentTimeClient::OnCharacteristicDiscoveryEvent(uint16_t conn_handle,
                                                      const ble_gatt_error* error,
                                                      const ble_gatt_chr* characteristic) {
  if (characteristic == nullptr && error->status == BLE_HS_EDONE) {
    if (isCharacteristicDiscovered) {
      NRF_LOG_INFO("CTS Characteristic discovery complete, fetching time");
      ble_gattc_read(conn_handle, currentTimeHandle, CurrentTimeReadCallback, this);
    } else {
      NRF_LOG_INFO("CTS Characteristic discovery unsuccessful");
      onServiceDiscovered(conn_handle);
    }

    return 0;
  }

  if (characteristic != nullptr && ble_uuid_cmp(&currentTimeCharacteristicUuid.u, &characteristic->uuid.u) == 0) {
    NRF_LOG_INFO("CTS Characteristic discovered : 0x%x", characteristic->val_handle);
    isCharacteristicDiscovered = true;
    currentTimeHandle = characteristic->val_handle;
  }
  return 0;
}

int CurrentTimeClient::OnCurrentTimeReadResult(uint16_t conn_handle, const ble_gatt_error* error, const ble_gatt_attr* attribute) {
  if (error->status == 0) {
    // TODO check that attribute->handle equals the handle discovered in OnCharacteristicDiscoveryEvent
    CtsData result;
    os_mbuf_copydata(attribute->om, 0, sizeof(CtsData), &result);
    uint16_t year = ((uint16_t) result.year_MSO << 8) + result.year_LSO;

    NRF_LOG_INFO("Received data: %d-%d-%d %d:%d:%d", year, result.month, result.dayofmonth, result.hour, result.minute, result.second);
    dateTimeController.SetTime(year, result.month, result.dayofmonth, result.hour, result.minute, result.second);
  } else {
    NRF_LOG_INFO("Error retrieving current time: %d", error->status);
  }

  onServiceDiscovered(conn_handle);
  return 0;
}

void CurrentTimeClient::Reset() {
  isDiscovered = false;
  isCharacteristicDiscovered = false;
}

void CurrentTimeClient::Discover(uint16_t connectionHandle, std::function<void(uint16_t)> onServiceDiscovered) {
  NRF_LOG_INFO("[CTS] Starting discovery");
  this->onServiceDiscovered = onServiceDiscovered;
  ble_gattc_disc_svc_by_uuid(connectionHandle, &ctsServiceUuid.u, OnDiscoveryEventCallback, this);
}
