#pragma once

#include <cstdint>

#include <FreeRTOS.h>

#include "components/ble/MotionService.h"

namespace Pinetime {
  namespace Controllers {
    class MotionController {
    public:
      void Update(int16_t x, int16_t y, int16_t z, uint32_t nbSteps);

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
      bool ShouldRaiseWake(bool isSleeping);

      int32_t CurrentShakeSpeed() const {
        return accumulatedSpeed;
      }

    private:
      uint32_t nbSteps = 0;
      uint32_t currentTripSteps = 0;

      TickType_t lastTime = 0;
      TickType_t time = 0;

      int16_t lastX = 0;
      int16_t x = 0;
      int16_t lastYForRaiseWake = 0;
      int16_t lastY = 0;
      int16_t y = 0;
      int16_t lastZ = 0;
      int16_t z = 0;
      int32_t accumulatedSpeed = 0;
    };
  }
}
