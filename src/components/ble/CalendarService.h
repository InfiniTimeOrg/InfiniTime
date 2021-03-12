#pragma once

// 00000000-78fc-48fe-8e23-433b3a1942d0
#include <host/ble_gap.h>
#include <host/ble_uuid.h>
#include <array>
#include "CalendarManager.h"

#define CALENDAR_SERVICE_UUID_BASE                                                                                                         \
  { 0xd0, 0x42, 0x19, 0x3a, 0x3b, 0x43, 0x23, 0x8e, 0xfe, 0x48, 0xfc, 0x78, 0x00, 0x00, 0x04, 0x00 }

namespace Pinetime {
  namespace System {
    class SystemTask;
  }
  namespace Controllers {

    class CalendarService {
    public:
      explicit CalendarService(Pinetime::System::SystemTask& system, CalendarManager& calendarManager);

      void Init();

      int OnCommand(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt);

      void event(char event);

      static const char EVENT_CALENDAR_ADD = 0x01;
      static const char EVENT_CALENDAR_DELETE = 0x02;

    private:
      static constexpr uint8_t calId[2] = {0x04, 0x00};
      static constexpr uint8_t calAddEventCharId[2] = {0x01, 0x00};
      static constexpr uint8_t calDeleteEventCharId[2] = {0x02, 0x00};
      ble_uuid128_t calUuid {
        .u = {.type = BLE_UUID_TYPE_128},
        .value = CALENDAR_SERVICE_UUID_BASE,
      };
      ble_uuid128_t calAddEventUuid {
        .u = {.type = BLE_UUID_TYPE_128},
        .value = CALENDAR_SERVICE_UUID_BASE,
      };
      ble_uuid128_t calDeleteEventUuid {
        .u = {.type = BLE_UUID_TYPE_128},
        .value = CALENDAR_SERVICE_UUID_BASE,
      };

      struct ble_gatt_chr_def characteristicDefinition[3];
      struct ble_gatt_svc_def serviceDefinition[2];

      Pinetime::System::SystemTask& m_system;
      CalendarManager& m_calendarManager;
    };
  }
};
