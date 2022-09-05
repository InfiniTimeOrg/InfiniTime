#pragma once

#include <cstdint>
#include <drivers/Bma421.h>
#include <components/ble/MotionService.h>

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
        if (stepCounterMustBeReset) {
          return 0;
        } else {
          return nbSteps;
        }
      }

      void ResetTrip() {
        currentTripSteps = 0;
      }
      uint32_t GetTripSteps() const {
        return currentTripSteps;
      }

      bool GetStepCounterMustBeReset() const {
        return stepCounterMustBeReset;
      }
      void SetStepCounterMustBeReset(bool reset) {
        stepCounterMustBeReset = reset;
      }

      bool Should_ShakeWake(uint16_t thresh);
      bool Should_RaiseWake(bool isSleeping);
      int32_t currentShakeSpeed();
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
      bool stepCounterMustBeReset = false;
      int16_t x;
      int16_t y;
      int16_t z;
      int16_t lastYForWakeUp = 0;
      bool isSensorOk = false;
      DeviceTypes deviceType = DeviceTypes::Unknown;
      Pinetime::Controllers::MotionService* service = nullptr;

      int16_t lastXForShake = 0;
      int16_t lastYForShake = 0;
      int16_t lastZForShake = 0;
      int32_t accumulatedspeed = 0;
      uint32_t lastShakeTime = 0;
    };
  }
}
