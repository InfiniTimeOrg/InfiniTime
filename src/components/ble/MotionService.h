#pragma once
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#undef max
#undef min

namespace Pinetime {
  namespace Controllers {
    class NimbleController;
    class MotionController;

    class MotionService {
    public:
      MotionService(NimbleController& nimble, Controllers::MotionController& motionController);
      void Init();
      int OnStepCountRequested(uint16_t attributeHandle, ble_gatt_access_ctxt* context);
      void OnNewStepCountValue(uint32_t stepCount);
      void OnNewMotionValues(int16_t x, int16_t y, int16_t z);

      void SubscribeNotification(uint16_t attributeHandle);
      void UnsubscribeNotification(uint16_t attributeHandle);

    private:
      NimbleController& nimble;
      Controllers::MotionController& motionController;

      struct ble_gatt_chr_def characteristicDefinition[3];
      struct ble_gatt_svc_def serviceDefinition[2];

      uint16_t stepCountHandle;
      uint16_t motionValuesHandle;
      bool stepCountNotificationEnabled {false};
      bool motionValuesNotificationEnabled {false};
    };
  }
}
