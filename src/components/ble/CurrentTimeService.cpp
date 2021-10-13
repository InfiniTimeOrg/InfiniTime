#include "components/ble/CurrentTimeService.h"
#include <hal/nrf_rtc.h>
#include <nrf_log.h>

using namespace Pinetime::Controllers;

constexpr ble_uuid16_t CurrentTimeService::ctsUuid;
constexpr ble_uuid16_t CurrentTimeService::ctChrUuid;

int CTSCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt, void* arg) {
  auto cts = static_cast<CurrentTimeService*>(arg);
  return cts->OnTimeAccessed(conn_handle, attr_handle, ctxt);
}

void CurrentTimeService::Init() {
  int res;
  res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);

  res = ble_gatts_add_svcs(serviceDefinition);
  ASSERT(res == 0);
}

int CurrentTimeService::OnTimeAccessed(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt) {

  NRF_LOG_INFO("Setting time...");

  if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
    CtsData result;
    os_mbuf_copydata(ctxt->om, 0, sizeof(CtsData), &result);

    NRF_LOG_INFO(
      "Received data: %d-%d-%d %d:%d:%d", result.year, result.month, result.dayofmonth, result.hour, result.minute, result.second);

    m_dateTimeController.SetTime(
      result.year, result.month, result.dayofmonth, 0, result.hour, result.minute, result.second, nrf_rtc_counter_get(portNRF_RTC_REG));

  } else if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
    CtsData currentDateTime;
    currentDateTime.year = m_dateTimeController.Year();
    currentDateTime.month = static_cast<u_int8_t>(m_dateTimeController.Month());
    currentDateTime.dayofmonth = m_dateTimeController.Day();
    currentDateTime.hour = m_dateTimeController.Hours();
    currentDateTime.minute = m_dateTimeController.Minutes();
    currentDateTime.second = m_dateTimeController.Seconds();
    currentDateTime.millis = 0;

    int res = os_mbuf_append(ctxt->om, &currentDateTime, sizeof(CtsData));
    return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
  }

  return 0;
}

CurrentTimeService::CurrentTimeService(DateTime& dateTimeController)
  : characteristicDefinition {{.uuid = &ctChrUuid.u,
                               .access_cb = CTSCallback,

                               .arg = this,
                               .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ},
                              {0}},
    serviceDefinition {
      {/* Device Information Service */
       .type = BLE_GATT_SVC_TYPE_PRIMARY,
       .uuid = &ctsUuid.u,
       .characteristics = characteristicDefinition},
      {0},
    },
    m_dateTimeController {dateTimeController} {
}
