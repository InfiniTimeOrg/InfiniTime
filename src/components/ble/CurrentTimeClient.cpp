/*  Copyright (C) 2020 JF

    This file is part of InfiniTime.

    InfiniTime is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    InfiniTime is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <hal/nrf_rtc.h>
#include <nrf_log.h>
#include "CurrentTimeClient.h"

using namespace Pinetime::Controllers;

constexpr ble_uuid16_t CurrentTimeClient::ctsServiceUuid;
constexpr ble_uuid16_t CurrentTimeClient::currentTimeCharacteristicUuid;

CurrentTimeClient::CurrentTimeClient(DateTime& dateTimeController) : dateTimeController{dateTimeController} {

}

void CurrentTimeClient::Init() {

}

bool CurrentTimeClient::OnDiscoveryEvent(uint16_t connectionHandle, const ble_gatt_error *error, const ble_gatt_svc *service) {
  if(service == nullptr && error->status == BLE_HS_EDONE) {
    NRF_LOG_INFO("CTS Discovery complete");
    return true;
  }
  
  if(service != nullptr && ble_uuid_cmp(((ble_uuid_t*)&ctsServiceUuid), &service->uuid.u) == 0) {
    NRF_LOG_INFO("CTS discovered : 0x%x", service->start_handle);
    isDiscovered = true;
    ctsStartHandle = service->start_handle;
    ctsEndHandle = service->end_handle;
    return false;
  }
  return false;
}

int CurrentTimeClient::OnCharacteristicDiscoveryEvent(uint16_t conn_handle, const ble_gatt_error *error,
                                                      const ble_gatt_chr *characteristic) {
  if(characteristic == nullptr && error->status == BLE_HS_EDONE) {
    NRF_LOG_INFO("CTS Characteristic discovery complete");
    return 0;
  }
  
  if(characteristic != nullptr && ble_uuid_cmp(((ble_uuid_t*)&currentTimeCharacteristicUuid), &characteristic->uuid.u) == 0) {
    NRF_LOG_INFO("CTS Characteristic discovered : 0x%x", characteristic->val_handle);
    currentTimeHandle = characteristic->val_handle;
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

bool CurrentTimeClient::IsDiscovered() const {
  return isDiscovered;
}

uint16_t CurrentTimeClient::StartHandle() const {
  return ctsStartHandle;
}

uint16_t CurrentTimeClient::EndHandle() const {
  return ctsEndHandle;
}

uint16_t CurrentTimeClient::CurrentTimeHandle() const {
  return currentTimeHandle;
}