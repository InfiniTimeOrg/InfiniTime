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
      void RunForDuration(uint16_t motorDuration);
      void StartRinging();
      void StopRinging();
      void StartAlarm();
      void StopAlarm();

    private:
      static void Ring(TimerHandle_t xTimer);
      static void AlarmRing(TimerHandle_t xTimer);
      static void StopMotor(TimerHandle_t xTimer);
      TimerHandle_t shortVib;
      TimerHandle_t longVib;
      TimerHandle_t alarmVib;
    };
  }
}
