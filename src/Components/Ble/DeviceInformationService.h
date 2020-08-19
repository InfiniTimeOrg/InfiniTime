#pragma once
#include <cstdint>
#include <array>

#include <host/ble_gap.h>

namespace Pinetime {
  namespace Controllers {
    class DeviceInformationService {
      public:
        DeviceInformationService();
        void Init();

        int OnDeviceInfoRequested(uint16_t conn_handle, uint16_t attr_handle,
                                    struct ble_gatt_access_ctxt *ctxt);

      private:
        static constexpr uint16_t deviceInfoId {0x180a};
        static constexpr uint16_t manufacturerNameId {0x2a29};
        static constexpr uint16_t modelNumberId {0x2a24};
        static constexpr uint16_t serialNumberId {0x2a25};
        static constexpr uint16_t fwRevisionId {0x2a26};
        static constexpr uint16_t hwRevisionId {0x2a27};

        static constexpr const char* manufacturerName = "Codingfield";
        static constexpr const char* modelNumber = "1";
        static constexpr const char* serialNumber = "9.8.7.6.5.4";
        static constexpr const char* fwRevision = "0.7.0";
        static constexpr const char* hwRevision = "1.0.0";

        static constexpr ble_uuid16_t deviceInfoUuid {
                .u { .type = BLE_UUID_TYPE_16 },
                .value = deviceInfoId
        };

        static constexpr ble_uuid16_t manufacturerNameUuid {
                .u { .type = BLE_UUID_TYPE_16 },
                .value = manufacturerNameId
        };

        static constexpr ble_uuid16_t modelNumberUuid {
                .u { .type = BLE_UUID_TYPE_16 },
                .value = modelNumberId
        };

        static constexpr ble_uuid16_t serialNumberUuid {
                .u { .type = BLE_UUID_TYPE_16 },
                .value = serialNumberId
        };

        static constexpr ble_uuid16_t fwRevisionUuid {
                .u { .type = BLE_UUID_TYPE_16 },
                .value = fwRevisionId
        };

        static constexpr ble_uuid16_t hwRevisionUuid {
                .u {.type = BLE_UUID_TYPE_16},
                .value = hwRevisionId
        };

        struct ble_gatt_chr_def characteristicDefinition[6];
        struct ble_gatt_svc_def serviceDefinition[2];


    };
  }
}