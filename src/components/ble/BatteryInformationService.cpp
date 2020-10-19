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
#include <nrf_log.h>
#include "BatteryInformationService.h"
#include "components/battery/BatteryController.h"

using namespace Pinetime::Controllers;

constexpr ble_uuid16_t BatteryInformationService::batteryInformationServiceUuid;
constexpr ble_uuid16_t BatteryInformationService::batteryLevelUuid;


int BatteryInformationServiceCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
  auto* batteryInformationService = static_cast<BatteryInformationService*>(arg);
  return batteryInformationService->OnBatteryServiceRequested(conn_handle, attr_handle, ctxt);
}

BatteryInformationService::BatteryInformationService(Controllers::Battery& batteryController) :
    batteryController{batteryController},
    characteristicDefinition{
        {
            .uuid = (ble_uuid_t *) &batteryLevelUuid,
            .access_cb = BatteryInformationServiceCallback,
            .arg = this,
            .flags = BLE_GATT_CHR_F_READ,
            .val_handle = &batteryLevelHandle
        },
        {
            0
        }
    },
    serviceDefinition{
        {
            /* Device Information Service */
            .type = BLE_GATT_SVC_TYPE_PRIMARY,
            .uuid = (ble_uuid_t *) &batteryInformationServiceUuid,
            .characteristics = characteristicDefinition
        },
        {
            0
        },
    }{
  
}

void BatteryInformationService::Init() {
  int res = 0;
  res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);
  
  res = ble_gatts_add_svcs(serviceDefinition);
  ASSERT(res == 0);
}

int BatteryInformationService::OnBatteryServiceRequested(uint16_t connectionHandle, uint16_t attributeHandle,
                                                         ble_gatt_access_ctxt *context) {
  if(attributeHandle == batteryLevelHandle) {
    NRF_LOG_INFO("BATTERY : handle = %d", batteryLevelHandle);
    static uint8_t batteryValue = batteryController.PercentRemaining();
    int res = os_mbuf_append(context->om, &batteryValue, 1);
    return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
  }
  return 0;
}