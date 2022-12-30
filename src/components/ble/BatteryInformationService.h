#pragma once
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#undef max
#undef min

namespace Pinetime {
  namespace System {
    class SystemTask;
  }

  namespace Controllers {
    class Battery;

    class BatteryInformationService {
    public:
      BatteryInformationService(Controllers::Battery& batteryController);
      void Init();

      int OnBatteryServiceRequested(uint16_t attributeHandle, ble_gatt_access_ctxt* context);
      void NotifyBatteryLevel(uint16_t connectionHandle, uint8_t level);

    private:
      Controllers::Battery& batteryController;
      static constexpr uint16_t batteryInformationServiceId {0x180F};
      static constexpr uint16_t batteryLevelId {0x2A19};

      static constexpr ble_uuid16_t batteryInformationServiceUuid {.u {.type = BLE_UUID_TYPE_16}, .value = batteryInformationServiceId};

      static constexpr ble_uuid16_t batteryLevelUuid {.u {.type = BLE_UUID_TYPE_16}, .value = batteryLevelId};

      struct ble_gatt_chr_def characteristicDefinition[3];
      struct ble_gatt_svc_def serviceDefinition[2];

      uint16_t batteryLevelHandle;
    };
  }
}
