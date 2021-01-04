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
        CurrentTimeService(DateTime &dateTimeController);
        void Init();

        int OnTimeAccessed(uint16_t conn_handle, uint16_t attr_handle,
                                    struct ble_gatt_access_ctxt *ctxt);

      private:
        static constexpr uint16_t ctsId {0x1805};
        static constexpr uint16_t ctsCharId {0x2a2b};

        static constexpr ble_uuid16_t ctsUuid {
                .u { .type = BLE_UUID_TYPE_16 },
                .value = ctsId
        };

        static constexpr ble_uuid16_t ctChrUuid {
                .u { .type = BLE_UUID_TYPE_16 },
                .value = ctsCharId
        };

        struct ble_gatt_chr_def characteristicDefinition[2];
        struct ble_gatt_svc_def serviceDefinition[2];

        typedef struct __attribute__((packed)) {
          uint16_t year;
          uint8_t month;
          uint8_t dayofmonth;
          uint8_t hour;
          uint8_t minute;
          uint8_t second;
          uint8_t millis;
          uint8_t reason;
        } CtsData;

        DateTime &m_dateTimeController;
    };
  }
}
