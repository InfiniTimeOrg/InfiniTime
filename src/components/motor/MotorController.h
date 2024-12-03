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
      void StartWakeAlarm();
      void StopWakeAlarm();
      void GradualWakeBuzz();
      void StopGradualWakeBuzz();
      void SetMotorStrength(uint8_t strength);

      uint8_t wakeAlarmStrength = 80;
      uint16_t wakeAlarmDuration = 100;
      uint8_t infiniSleepMotorStrength = 100;

    private:
      static void Ring(TimerHandle_t xTimer);
      static void WakeAlarmRing(TimerHandle_t xTimer);
      static void StopMotor(TimerHandle_t xTimer);

      TimerHandle_t shortVib;
      TimerHandle_t longVib;

      TimerHandle_t wakeAlarmVib;
    };
  }
}
