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
      void RunForDuration(uint8_t motorDuration);
      void StartRinging();
      void StopRinging();
      void VibrateTune();
      void RunTest();

    private:
      static void Ring(TimerHandle_t xTimer);
      static void StopMotor(TimerHandle_t xTimer);
      static void Vibrate(TimerHandle_t xTimer);

      void StopTune();
      void StartTune();
      void ScheduleVibrateTimer(uint8_t motorDuration, bool vibrate);

      TimerHandle_t shortVib;
      TimerHandle_t longVib;
      uint8_t step;

    };
  }
}
