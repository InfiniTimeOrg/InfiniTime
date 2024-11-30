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
      void StartWakeAlarm();
      void StopWakeAlarm();
      void GradualWakeBuzz();
      void StopGradualWakeBuzz();
      void SetMotorStrength(uint8_t strength);

      uint8_t wakeAlarmStrength = 80;
      uint16_t wakeAlarmDuration = 100;

    private:
      static void Ring(TimerHandle_t xTimer);
      static void AlarmRing(TimerHandle_t xTimer);
      static void WakeAlarmRing(TimerHandle_t xTimer);
      static void StopMotor(TimerHandle_t xTimer);
      static void GradualWakeBuzzRing(TimerHandle_t xTimer);
      static void StopGradualWakeBuzzCallback(TimerHandle_t xTimer);

      TimerHandle_t shortVib;
      TimerHandle_t longVib;
      TimerHandle_t alarmVib;
      TimerHandle_t gradualWakeBuzzDelay;
      TimerHandle_t gradualWakeBuzzEnd;

      TimerHandle_t wakeAlarmVib;
    };
  }
}
