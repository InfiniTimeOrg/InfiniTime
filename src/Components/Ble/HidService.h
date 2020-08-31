#pragma once
#include <host/ble_gap.h>

namespace Pinetime {
  namespace System {
    class SystemTask;
  }
  namespace Controllers {
    class HidService {
      public:
        HidService();
        void Init();

        int OnHidServiceRequested(uint16_t connectionHandle, uint16_t attributeHandle, ble_gatt_access_ctxt *context);

        int
        OnReportDescriptorRequested(uint16_t connectionHandle, uint16_t attributeHandle, ble_gatt_access_ctxt *context);

        int OnReportMapDescriptorRequested(uint16_t connectionHandle, uint16_t attributeHandle,
                                           ble_gatt_access_ctxt *context);

        void Test();

      private:
        static constexpr uint16_t hidServiceId {0x1812};
        static constexpr uint16_t protocolModeId {0x2A4E};
        static constexpr uint16_t reportId {0x2A4D};
        static constexpr uint16_t reportMapId {0x2A4B};
        static constexpr uint16_t bootKeyboardInputReportId {0x2A22};
        static constexpr uint16_t bootKeyboardOutputReportId {0x2A32};
        static constexpr uint16_t bootMouseInputReportId {0x2A33};
        static constexpr uint16_t informationId {0x2A4A};
        static constexpr uint16_t controlPointId {0x2A4C};

        static constexpr uint16_t descriptorId {0x2908};
        static constexpr uint16_t reportMapDescriptorId {0x2907};

        static constexpr ble_uuid16_t hidServiceUuid {
                .u {.type = BLE_UUID_TYPE_16},
                .value = hidServiceId
        };

        static constexpr ble_uuid16_t protocolModeUuid {
                .u {.type = BLE_UUID_TYPE_16},
                .value = protocolModeId
        };

        static constexpr ble_uuid16_t reportUuid {
                .u {.type = BLE_UUID_TYPE_16},
                .value = reportId
        };

        static constexpr ble_uuid16_t reportMapUuid {
                .u {.type = BLE_UUID_TYPE_16},
                .value = reportMapId
        };

        static constexpr ble_uuid16_t bootKeyboardInputReportUuid {
                .u {.type = BLE_UUID_TYPE_16},
                .value = bootKeyboardInputReportId
        };

        static constexpr ble_uuid16_t bootKeyboardOutputReportUuid {
                .u {.type = BLE_UUID_TYPE_16},
                .value = bootKeyboardOutputReportId
        };

        static constexpr ble_uuid16_t bootMouseInputReportUuid {
                .u {.type = BLE_UUID_TYPE_16},
                .value = bootMouseInputReportId
        };

        static constexpr ble_uuid16_t informationUuid {
                .u {.type = BLE_UUID_TYPE_16},
                .value = informationId
        };

        static constexpr ble_uuid16_t controlPointUuid {
                .u {.type = BLE_UUID_TYPE_16},
                .value = controlPointId
        };

        static constexpr ble_uuid16_t descriptorUuid {
                .u {.type = BLE_UUID_TYPE_16},
                .value = descriptorId
        };


        static constexpr ble_uuid16_t descriptorMapUuid {
                .u {.type = BLE_UUID_TYPE_16},
                .value = reportMapDescriptorId
        };


        struct ble_gatt_dsc_def reportInDescriptorDefinition[2];
        struct ble_gatt_dsc_def reportOutDescriptorDefinition[2];
        struct ble_gatt_dsc_def reportDescriptorDefinition[2];
        struct ble_gatt_dsc_def reportMapDescriptorDefinitions[2];
        struct ble_gatt_chr_def characteristicDefinition[9];
        struct ble_gatt_svc_def serviceDefinition[2];

        uint16_t protocolModeHandle;
        uint16_t reportHandle;
        uint16_t reportMapHandle;
        uint16_t informationHandle;
        uint16_t controlPointHandle;

        char *OperationToString(uint8_t op);
    };
  }
}

