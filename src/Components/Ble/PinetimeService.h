#pragma once
#include <cstdint>
#include <array>
#include <Components/DateTime/DateTimeController.h>
#include <host/ble_gap.h>

namespace Pinetime {
  namespace Controllers {
    class PinetimeService {
      public:
        PinetimeService();
        void Init();

        int OnTimeAccessed(uint16_t conn_handle, uint16_t attr_handle,
                                    struct ble_gatt_access_ctxt *ctxt);

        void setDateTimeController(DateTime *dateTimeController);

      private:
        static constexpr uint16_t pinetimeId {0x6666};
        static constexpr uint16_t timeCharId {0x6667};

        static constexpr ble_uuid16_t pinetimeUuid {
                .u { .type = BLE_UUID_TYPE_16 },
                .value = pinetimeId
        };

        static constexpr ble_uuid16_t timeUuid {
                .u { .type = BLE_UUID_TYPE_16 },
                .value = timeCharId
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

        DateTime *m_dateTimeController = nullptr;
    };
  }
}
