#pragma once
#include <cstdint>
#include <array>

#include "components/datetime/DateTimeController.h"
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#undef max
#undef min

namespace Pinetime {
  namespace Controllers {
    class CurrentTimeService {
    public:
      CurrentTimeService(DateTime& dateTimeController);
      void Init();

      int OnCurrentTimeServiceAccessed(struct ble_gatt_access_ctxt* ctxt);
      int OnCurrentTimeAccessed(struct ble_gatt_access_ctxt* ctxt);
      int OnLocalTimeAccessed(struct ble_gatt_access_ctxt* ctxt);

    private:
      static constexpr uint16_t ctsId {0x1805};
      static constexpr uint16_t ctsCurrentTimeCharId {0x2a2b};
      static constexpr uint16_t ctsLocalTimeCharId {0x2a0f};

      static constexpr ble_uuid16_t ctsUuid {.u {.type = BLE_UUID_TYPE_16}, .value = ctsId};

      static constexpr ble_uuid16_t ctsCtChrUuid {.u {.type = BLE_UUID_TYPE_16}, .value = ctsCurrentTimeCharId};
      static constexpr ble_uuid16_t ctsLtChrUuid {.u {.type = BLE_UUID_TYPE_16}, .value = ctsLocalTimeCharId};

      struct ble_gatt_chr_def characteristicDefinition[3];
      struct ble_gatt_svc_def serviceDefinition[2];

      typedef struct __attribute__((packed)) {
        uint8_t year_LSO; // BLE GATT is little endian
        uint8_t year_MSO;
        uint8_t month;
        uint8_t dayofmonth;
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
        uint8_t dayofweek;
        uint8_t fractions256; // currently ignored
        uint8_t reason;       // currently ignored, not that any host would set it anyway
      } CtsCurrentTimeData;

      typedef struct __attribute__((packed)) {
        int8_t timezone;
        int8_t dst;
      } CtsLocalTimeData;

      DateTime& m_dateTimeController;
    };
  }
}
