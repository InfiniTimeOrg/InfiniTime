#pragma once

#include <FreeRTOS.h>
#include <timers.h>
#include <cstdint>

namespace Pinetime {
  namespace Controllers {

    class MotorController {
    public:
      MotorController() = default;

      void Init();

      // SingleVibration and StartRinging do nothing when a vibration is already running.
      // they return false in this case.
      // if everything went good they return true.
      bool SingleVibration(uint8_t Duration);
      bool StartRinging(uint8_t Duration);
      void StopRinging();

      bool IsVibrating() {
        return isShortVibrating || isLongVibrating;
      };

    private:
      void RunForDuration(uint8_t motorDuration);

      static void Ring(TimerHandle_t xTimer);
      static void StopMotor(TimerHandle_t xTimer);
      TimerHandle_t shortVib;
      TimerHandle_t longVib;

      bool isShortVibrating = false;
      bool isLongVibrating = false;
    };
  }
}
