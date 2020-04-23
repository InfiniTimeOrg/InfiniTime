#pragma once
#include <cstdint>
#include <array>
#include <Components/DateTime/DateTimeController.h>
#include <host/ble_gap.h>

namespace Pinetime {
  namespace Controllers {
    int CurrentTimeDiscoveryEventCallback(uint16_t conn_handle, const struct ble_gatt_error *error,
                               const struct ble_gatt_svc *service, void *arg);
    int CurrentTimeCharacteristicDiscoveredCallback(uint16_t conn_handle, const struct ble_gatt_error *error,
                                         const struct ble_gatt_chr *chr, void *arg);
    int CurrentTimeReadCallback(uint16_t conn_handle, const struct ble_gatt_error *error,
                                       struct ble_gatt_attr *attr, void *arg);

    class CurrentTimeClient {
      public:
        explicit CurrentTimeClient(DateTime& dateTimeController);
        void Init();
        int OnDiscoveryEvent(uint16_t connectionHandle, const ble_gatt_error *error, const ble_gatt_svc *service);
        int OnCharacteristicDiscoveryEvent(uint16_t conn_handle, const ble_gatt_error *error,
                                                             const ble_gatt_chr *characteristic);
        int OnCurrentTimeReadResult(uint16_t conn_handle, const ble_gatt_error *error, const ble_gatt_attr *attribute);


        void StartDiscovery(uint16_t connectionHandle);
      private:
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

        static constexpr uint16_t ctsServiceId {0x1805};
        static constexpr uint16_t currentTimeCharacteristicId {0x2a2b};

        static constexpr ble_uuid16_t ctsServiceUuid {
                .u { .type = BLE_UUID_TYPE_16 },
                .value = ctsServiceId
        };
        static constexpr ble_uuid16_t currentTimeCharacteristicUuid {
                .u { .type = BLE_UUID_TYPE_16 },
                .value = currentTimeCharacteristicId
        };

        DateTime& dateTimeController;
    };
  }
}