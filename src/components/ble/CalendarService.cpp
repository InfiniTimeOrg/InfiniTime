#include <nrf_log.h>
#include "CalendarService.h"

using namespace Pinetime::Controllers;

int CalCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt, void* arg) {
  auto pCalendarEventService = static_cast<Pinetime::Controllers::CalendarService*>(arg);
  return pCalendarEventService->OnCommand(conn_handle, attr_handle, ctxt);
}

CalendarService::CalendarService(Pinetime::System::SystemTask& system, CalendarManager& calendarManager)
  : m_system(system), m_calendarManager(calendarManager) {
  calUuid.value[14] = calId[0];
  calUuid.value[15] = calId[1];

  calAddEventUuid.value[12] = calAddEventCharId[0];
  calAddEventUuid.value[13] = calAddEventCharId[1];
  calAddEventUuid.value[14] = calId[0];
  calAddEventUuid.value[15] = calId[1];

  calDeleteEventUuid.value[12] = calDeleteEventCharId[0];
  calDeleteEventUuid.value[13] = calDeleteEventCharId[1];
  calDeleteEventUuid.value[14] = calId[0];
  calDeleteEventUuid.value[15] = calId[1];

  characteristicDefinition[0] = {
    .uuid = (ble_uuid_t*) (&calAddEventUuid),
    .access_cb = CalCallback,
    .arg = this,
    .flags = BLE_GATT_CHR_F_WRITE,
  };
  characteristicDefinition[1] = {
    .uuid = (ble_uuid_t*) (&calDeleteEventUuid),
    .access_cb = CalCallback,
    .arg = this,
    .flags = BLE_GATT_CHR_F_WRITE,
  };
  characteristicDefinition[2] = {0};

  serviceDefinition[0] = {
    .type = BLE_GATT_SVC_TYPE_PRIMARY,
    .uuid = (ble_uuid_t*) &calUuid,
    .characteristics = characteristicDefinition,
  };
  serviceDefinition[1] = {0};
}
void CalendarService::Init() {
  int res = 0;
  res = ble_gatts_count_cfg(serviceDefinition);
  ASSERT(res == 0);

  res = ble_gatts_add_svcs(serviceDefinition);
  ASSERT(res == 0);
}
int CalendarService::OnCommand(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt) {
  if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
    size_t notifSize = OS_MBUF_PKTLEN(ctxt->om);
    uint8_t data[notifSize + 1];
    data[notifSize] = '\0';
    os_mbuf_copydata(ctxt->om, 0, notifSize, data);
    char* s = (char*) &data[0];
    NRF_LOG_INFO("DATA : %s", s);
    if (ble_uuid_cmp(ctxt->chr->uuid, (ble_uuid_t*) &calAddEventUuid) == 0) {
      CalendarManager::CalendarEvent event {
        .id = s[0],
        .title = &s[1],
      };
      m_calendarManager.addEvent(event);
    } else if (ble_uuid_cmp(ctxt->chr->uuid, (ble_uuid_t*) &calDeleteEventUuid) == 0) {
      m_calendarManager.deleteEvent(s[0]);
    }
  }
  return 0;
}
