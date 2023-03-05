#pragma once

#include <cstdint>

#include "drivers/Bma421.h"
#include "components/ble/MotionService.h"

namespace Pinetime {
  namespace Controllers {
    class MotionController {
    public:
      enum class DeviceTypes {
        Unknown,
        BMA421,
        BMA425,
      };

      void Update(int16_t x, int16_t y, int16_t z, uint32_t nbSteps);

      int16_t X() const {
        return x;
      }

      int16_t Y() const {
        return y;
      }

      int16_t Z() const {
        return z;
      }

      uint32_t NbSteps() const {
        return nbSteps;
      }

      void ResetTrip() {
        currentTripSteps = 0;
      }

      uint32_t GetTripSteps() const {
        return currentTripSteps;
      }

      bool ShouldShakeWake(uint16_t thresh);
      bool Should_RaiseWake(bool isSleeping);

      int32_t CurrentShakeSpeed() const {
        return accumulatedSpeed;
      }

      void IsSensorOk(bool isOk);

      bool IsSensorOk() const {
        return isSensorOk;
      }

      DeviceTypes DeviceType() const {
        return deviceType;
      }

      void Init(Pinetime::Drivers::Bma421::DeviceTypes types);
      void SetService(Pinetime::Controllers::MotionService* service);

    private:
      uint32_t nbSteps;
      uint32_t currentTripSteps = 0;

      int16_t x;
      int16_t lastYForWakeUp = 0;
      int16_t lastY = 0;
      int16_t y;
      int16_t lastZ = 0;
      int16_t z;

      bool isSensorOk = false;
      DeviceTypes deviceType = DeviceTypes::Unknown;
      Pinetime::Controllers::MotionService* service = nullptr;

      int32_t accumulatedSpeed = 0;
      uint32_t lastShakeTime = 0;
    };
  }
}
