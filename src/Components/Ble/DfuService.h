#pragma once
#include <cstdint>
#include <array>

#include <host/ble_gap.h>

namespace Pinetime {
  namespace Controllers {
    class DfuService {
      public:
        DfuService();
        void Init();

        int OnServiceData(uint16_t connectionHandle, uint16_t attributeHandle, ble_gatt_access_ctxt *context);
      private:
        static constexpr uint16_t dfuServiceId {0x1530};
        static constexpr uint16_t packetCharacteristicId {0x1532};
        static constexpr uint16_t controlPointCharacteristicId {0x1531};
        static constexpr uint16_t revisionCharacteristicId {0x1534};

        uint16_t revision {0x0008};

        static constexpr uint16_t opcodeInit = 0x0002;

        static constexpr ble_uuid128_t serviceUuid {
                .u { .type = BLE_UUID_TYPE_128},
                .value = {0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15,
                          0xDE, 0xEF, 0x12, 0x12, 0x30, 0x15, 0x00, 0x00}
        };

        static constexpr ble_uuid128_t packetCharacteristicUuid {
                .u { .type = BLE_UUID_TYPE_128},
                .value = {0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15,
                          0xDE, 0xEF, 0x12, 0x12, 0x32, 0x15, 0x00, 0x00}
        };

        static constexpr ble_uuid128_t controlPointCharacteristicUuid {
                .u { .type = BLE_UUID_TYPE_128},
                .value = {0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15,
                          0xDE, 0xEF, 0x12, 0x12, 0x31, 0x15, 0x00, 0x00}
        };

        static constexpr ble_uuid128_t revisionCharacteristicUuid {
                .u { .type = BLE_UUID_TYPE_128},
                .value = {0x23, 0xD1, 0xBC, 0xEA, 0x5F, 0x78, 0x23, 0x15,
                          0xDE, 0xEF, 0x12, 0x12, 0x34, 0x15, 0x00, 0x00}
        };

        struct ble_gatt_chr_def characteristicDefinition[4];
        struct ble_gatt_svc_def serviceDefinition[2];
        uint16_t packetCharacteristicHandle;
        uint16_t controlPointCharacteristicHandle;
        uint16_t revisionCharacteristicHandle;
        uint8_t opcode = 0;
        uint8_t param = 0;
        uint8_t nbPacketsToNotify = 0;
        uint32_t nbPacketReceived = 0;
        bool dataMode = false;
        uint32_t bytesReceived = 0;
    };
  }
}