#pragma once

#include <cstdint>

#include <FreeRTOS.h>

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
      bool ShouldRaiseWake(bool isSleeping);

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

      DeviceTypes deviceType = DeviceTypes::Unknown;
      Pinetime::Controllers::MotionService* service = nullptr;
    };
  }
}
