#include "PinetimeService.h"
#include <hal/nrf_rtc.h>

using namespace Pinetime::Controllers;

constexpr ble_uuid16_t PinetimeService::pinetimeUuid;
constexpr ble_uuid16_t PinetimeService::timeUuid;


int PinetimeTimeCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg) {
  auto pinetimeService = static_cast<PinetimeService*>(arg);
  return pinetimeService->OnTimeAccessed(conn_handle, attr_handle, ctxt);
}

void PinetimeService::Init() {
  ble_gatts_count_cfg(serviceDefinition);
  ble_gatts_add_svcs(serviceDefinition);
}


int PinetimeService::OnTimeAccessed(uint16_t conn_handle, uint16_t attr_handle,
                                                    struct ble_gatt_access_ctxt *ctxt) {

    NRF_LOG_INFO("Setting time...");

  if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
      if (m_dateTimeController) {
          CtsData result;
          os_mbuf_copydata(ctxt->om, 0, sizeof(CtsData), &result);

          NRF_LOG_INFO("Received data: %d-%d-%d %d:%d:%d", result.year,
                 result.month, result.dayofmonth,
                 result.hour, result.minute, result.second);

          m_dateTimeController->SetTime(result.year, result.month, result.dayofmonth,
                               0, result.hour, result.minute, result.second, nrf_rtc_counter_get(portNRF_RTC_REG));
      }
  }
  return 0;
}

PinetimeService::PinetimeService() :
        characteristicDefinition{
                {
                        .uuid = (ble_uuid_t *) &timeUuid,
                        .access_cb = PinetimeTimeCallback,

                        .arg = this,
                        .flags = BLE_GATT_CHR_F_WRITE
                },
                {
                  0
                }
        },
        serviceDefinition{
                {
                        /* Device Information Service */
                        .type = BLE_GATT_SVC_TYPE_PRIMARY,
                        .uuid = (ble_uuid_t *) &pinetimeUuid,
                        .characteristics = characteristicDefinition
                },
                {
                        0
                },
        }
         {

}

void PinetimeService::setDateTimeController(DateTime *dateTimeController)
{
        m_dateTimeController = dateTimeController;
}
