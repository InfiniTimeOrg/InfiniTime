#pragma once
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#undef max
#undef min
#include <cstdint>
#include "BleClient.h"

namespace Pinetime {
    namespace Controllers {
        class DateTime;

        class CurrentTimeClient : public BleClient {
        public:
            explicit CurrentTimeClient(DateTime& dateTimeController);
            void Init();
            void Reset();
            bool OnDiscoveryEvent(uint16_t connectionHandle, const ble_gatt_error *error, const ble_gatt_svc *service);
            int OnCharacteristicDiscoveryEvent(uint16_t conn_handle, const ble_gatt_error *error,
                                               const ble_gatt_chr *characteristic);
            int OnCurrentTimeReadResult(uint16_t conn_handle, const ble_gatt_error *error, const ble_gatt_attr *attribute);
            static constexpr const ble_uuid16_t* Uuid() { return &CurrentTimeClient::ctsServiceUuid; }
            static constexpr const ble_uuid16_t* CurrentTimeCharacteristicUuid() { return &CurrentTimeClient::currentTimeCharacteristicUuid; }
            void Discover(uint16_t connectionHandle, std::function<void(uint16_t)> lambda) override;

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
            bool isDiscovered = false;
            uint16_t ctsStartHandle;
            uint16_t ctsEndHandle;

            bool isCharacteristicDiscovered = false;
            uint16_t currentTimeHandle;
            std::function<void(uint16_t)> onServiceDiscovered;
        };
    }
}