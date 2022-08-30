#pragma once

#include <cstdint>
#include <drivers/Bma421.h>
#include <components/ble/MotionService.h>
#include <functional>

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

      bool shouldRaiseWake() const {
        return callbackShouldRaiseWake();
      }

      bool Should_ShakeWake(uint16_t thresh);
      int32_t currentShakeSpeed() const;
      void IsSensorOk(bool isOk);
      bool IsSensorOk() const {
        return isSensorOk;
      }

      DeviceTypes DeviceType() const {
        return deviceType;
      }

      void Init(Pinetime::Drivers::Bma421::DeviceTypes types, Pinetime::Drivers::Bma421* bma);
      void SetService(Pinetime::Controllers::MotionService* service);

    private:
      uint32_t nbSteps = 0;
      uint32_t currentTripSteps = 0;
      int16_t x = 0;
      int16_t y = 0;
      int16_t z = 0;
      int16_t lastYForWakeUp = 0;
      bool isSensorOk = false;
      DeviceTypes deviceType = DeviceTypes::Unknown;
      Pinetime::Controllers::MotionService* service = nullptr;
      Pinetime::Drivers::Bma421* bmaDriver = nullptr;

      int16_t lastXForShake = 0;
      int16_t lastYForShake = 0;
      int16_t lastZForShake = 0;
      int32_t accumulatedspeed = 0;
      uint32_t lastShakeTime = 0;

      // Use the software check by default. Optionally, this can be overriden during runtime, without the SystemTask's knowledge.
      // Using std::function will allow for changing the algorithm during runtime. This is better than having many if statements select the
      // correct algorithm.
      // Currently, this will be overridden if the BMA chip is a BMA425
      std::function<bool()> callbackShouldRaiseWake = [this]() -> bool {
        return Should_RaiseWake();
      };

      bool Should_RaiseWake();
      bool BMA425ShouldRaiseWake() const;
    };
  }
}