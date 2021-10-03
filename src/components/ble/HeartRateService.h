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
    class HeartRateController;
    class HeartRateService {
    public:
      HeartRateService(Pinetime::System::SystemTask& system, Controllers::HeartRateController& heartRateController);
      void Init();
      int OnHeartRateRequested(uint16_t connectionHandle, uint16_t attributeHandle, ble_gatt_access_ctxt* context);
      void OnNewHeartRateValue(uint8_t hearRateValue);

    private:
      Pinetime::System::SystemTask& system;
      Controllers::HeartRateController& heartRateController;
      static constexpr uint16_t heartRateServiceId {0x180D};
      static constexpr uint16_t heartRateMeasurementId {0x2A37};

      static constexpr ble_uuid16_t heartRateServiceUuid {.u {.type = BLE_UUID_TYPE_16}, .value = heartRateServiceId};

      static constexpr ble_uuid16_t heartRateMeasurementUuid {.u {.type = BLE_UUID_TYPE_16}, .value = heartRateMeasurementId};

      struct ble_gatt_chr_def characteristicDefinition[3];
      struct ble_gatt_svc_def serviceDefinition[2];

      uint16_t heartRateMeasurementHandle;
    };
  }
}
