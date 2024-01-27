#pragma once

#include <cstdint>

#include <FreeRTOS.h>

#include "drivers/Bma421.h"
#include "components/ble/MotionService.h"
#include "utility/CircularBuffer.h"

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
        return yHistory[0];
      }

      int16_t Z() const {
        return zHistory[0];
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
      bool ShouldRaiseWake() const;
      bool ShouldLowerSleep() const;

      int32_t CurrentShakeSpeed() const {
        return accumulatedSpeed;
      }

      DeviceTypes DeviceType() const {
        return deviceType;
      }

      void Init(Pinetime::Drivers::Bma421::DeviceTypes types);

      void SetService(Pinetime::Controllers::MotionService* service) {
        this->service = service;
      }

      Pinetime::Controllers::MotionService* GetService() const {
        return service;
      }

    private:
      uint32_t nbSteps = 0;
      uint32_t currentTripSteps = 0;

      TickType_t lastTime = 0;
      TickType_t time = 0;

      struct AccelStats {
        static constexpr uint8_t numHistory = 2;

        int16_t yMean = 0;
        int16_t zMean = 0;
        int16_t prevYMean = 0;
        int16_t prevZMean = 0;

        uint32_t yVariance = 0;
        uint32_t zVariance = 0;
      };

      AccelStats GetAccelStats() const;

      AccelStats stats = {};

      int16_t lastX = 0;
      int16_t x = 0;
      static constexpr uint8_t histSize = 8;
      Utility::CircularBuffer<int16_t, histSize> yHistory = {};
      Utility::CircularBuffer<int16_t, histSize> zHistory = {};
      int32_t accumulatedSpeed = 0;

      DeviceTypes deviceType = DeviceTypes::Unknown;
      Pinetime::Controllers::MotionService* service = nullptr;
    };
  }
}
