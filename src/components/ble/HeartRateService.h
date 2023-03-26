#pragma once
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#include <atomic>
#undef max
#undef min

namespace Pinetime {
  namespace Controllers {
    class HeartRateController;
    class NimbleController;

    class HeartRateService {
    public:
      HeartRateService(NimbleController& nimble, Controllers::HeartRateController& heartRateController);
      void Init();
      int OnHeartRateRequested(uint16_t attributeHandle, ble_gatt_access_ctxt* context);
      void OnNewHeartRateValue(uint8_t hearRateValue);

      void SubscribeNotification(uint16_t attributeHandle);
      void UnsubscribeNotification(uint16_t attributeHandle);

    private:
      NimbleController& nimble;
      Controllers::HeartRateController& heartRateController;
      static constexpr uint16_t heartRateServiceId {0x180D};
      static constexpr uint16_t heartRateMeasurementId {0x2A37};

      static constexpr ble_uuid16_t heartRateServiceUuid {.u {.type = BLE_UUID_TYPE_16}, .value = heartRateServiceId};

      static constexpr ble_uuid16_t heartRateMeasurementUuid {.u {.type = BLE_UUID_TYPE_16}, .value = heartRateMeasurementId};

      struct ble_gatt_chr_def characteristicDefinition[2];
      struct ble_gatt_svc_def serviceDefinition[2];

      uint16_t heartRateMeasurementHandle;
      std::atomic_bool heartRateMeasurementNotificationEnable {false};
    };
  }
}
