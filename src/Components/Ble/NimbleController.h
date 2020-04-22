#pragma once

#include <cstdint>
#include <host/ble_gap.h>

namespace Pinetime {
  namespace Controllers {
    class DateTime;
    class NimbleController {
      public:
        NimbleController(DateTime& dateTimeController);
        void Init();
        void StartAdvertising();
        int OnGAPEvent(ble_gap_event *event);
        int OnDiscoveryEvent(uint16_t connectionHandle, const ble_gatt_error *error, const ble_gatt_svc *service);
        int
        OnCharacteristicDiscoveryEvent(uint16_t conn_handle, const ble_gatt_error *error,
                                       const ble_gatt_chr *characteristic);
        int OnCurrentTimeReadResult(uint16_t conn_handle, const ble_gatt_error *error, const ble_gatt_attr *attribute);
      private:
        static constexpr char* deviceName = "Pinetime-JF";
        static constexpr uint16_t BleGatServiceCts = 0x1805;

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

        DateTime& dateTimeController;

        ble_uuid16_t ctsUuid;

        static constexpr uint16_t bleGattCharacteristicCurrentTime = 0x2a2b;
        ble_uuid16_t ctsCurrentTimeUuid;

        uint8_t addrType;
        uint16_t connectionHandle;
    };
  }
}
