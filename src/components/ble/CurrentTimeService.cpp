#include "components/ble/CurrentTimeService.h"
#include <nrf_log.h>

using namespace Pinetime::Controllers;

constexpr ble_uuid16_t CurrentTimeService::ctsUuid;
constexpr ble_uuid16_t CurrentTimeService::ctsCtChrUuid;
constexpr ble_uuid16_t CurrentTimeService::ctsLtChrUuid;

int CTSCallback(uint16_t /*conn_handle*/, uint16_t /*attr_handle*/, struct ble_gatt_access_ctxt* ctxt, void* arg) {
  auto cts = static_cast<CurrentTimeService*>(arg);

  return cts->OnCurrentTimeServiceAccessed(ctxt);
}

int CurrentTimeService::OnCurrentTimeServiceAccessed(struct ble_gatt_access_ctxt* ctxt) {
  switch (ble_uuid_u16(ctxt->chr->uuid)) {
    case ctsCurrentTimeCharId:
      return OnCurrentTimeAccessed(ctxt);
    case ctsLocalTimeCharId:
      return OnLocalTimeAccessed(ctxt);
  }
  return -1; // Unknown characteristic
}

void CurrentTimeService::Init() {
  int res;
  res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);

  res = ble_gatts_add_svcs(serviceDefinition);

  ASSERT(res == 0);
}

int CurrentTimeService::OnCurrentTimeAccessed(struct ble_gatt_access_ctxt* ctxt) {

  NRF_LOG_INFO("Setting time...");

  if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
    CtsCurrentTimeData result;
    int res = os_mbuf_copydata(ctxt->om, 0, sizeof(CtsCurrentTimeData), &result);
    if (res < 0) {
      NRF_LOG_ERROR("Error reading BLE Data writing to CTS Current Time (too little data)")
      return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    uint16_t year = ((uint16_t) result.year_MSO << 8) + result.year_LSO;

    NRF_LOG_INFO("Received data: %d-%d-%d %d:%d:%d", year, result.month, result.dayofmonth, result.hour, result.minute, result.second);

    m_dateTimeController.SetTime(year, result.month, result.dayofmonth, result.hour, result.minute, result.second);

  } else if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
    CtsCurrentTimeData currentDateTime;
    currentDateTime.year_LSO = m_dateTimeController.Year() & 0xff;
    currentDateTime.year_MSO = (m_dateTimeController.Year() >> 8) & 0xff;
    currentDateTime.month = static_cast<u_int8_t>(m_dateTimeController.Month());
    currentDateTime.dayofmonth = m_dateTimeController.Day();
    currentDateTime.hour = m_dateTimeController.Hours();
    currentDateTime.minute = m_dateTimeController.Minutes();
    currentDateTime.second = m_dateTimeController.Seconds();
    currentDateTime.fractions256 = 0;

    int res = os_mbuf_append(ctxt->om, &currentDateTime, sizeof(CtsCurrentTimeData));
    return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
  }

  return 0;
}

int CurrentTimeService::OnLocalTimeAccessed(struct ble_gatt_access_ctxt* ctxt) {
  NRF_LOG_INFO("Setting timezone...");

  if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
    CtsLocalTimeData result;
    int res = os_mbuf_copydata(ctxt->om, 0, sizeof(CtsLocalTimeData), &result);

    if (res < 0) {
      NRF_LOG_ERROR("Error reading BLE Data writing to CTS Local Time (too little data)")
      return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
    }

    NRF_LOG_INFO("Received data: %d %d", result.timezone, result.dst);

    m_dateTimeController.SetTimeZone(result.timezone, result.dst);

  } else if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
    CtsLocalTimeData currentTimezone;
    currentTimezone.timezone = m_dateTimeController.TzOffset();
    currentTimezone.dst = m_dateTimeController.DstOffset();

    int res = os_mbuf_append(ctxt->om, &currentTimezone, sizeof(currentTimezone));
    return (res == 0) ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
  }

  return 0;
}

CurrentTimeService::CurrentTimeService(DateTime& dateTimeController)
  : characteristicDefinition {

                              {.uuid = &ctsLtChrUuid.u,
                               .access_cb = CTSCallback,
                               .arg = this,
                               .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ},

                               {.uuid = &ctsCtChrUuid.u,
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
