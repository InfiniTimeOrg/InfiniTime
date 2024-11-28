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
      void GradualWakeBuzz();
      void StopGradualWakeBuzz();

    private:
      static void Ring(TimerHandle_t xTimer);
      static void AlarmRing(TimerHandle_t xTimer);
      static void StopMotor(TimerHandle_t xTimer);
      static void GradualWakeBuzzRing(TimerHandle_t xTimer);
      static void StopGradualWakeBuzzCallback(TimerHandle_t xTimer);
      void InitPWM();
      void SetMotorStrength(uint8_t strength);

      TimerHandle_t shortVib;
      TimerHandle_t longVib;
      TimerHandle_t alarmVib;
      TimerHandle_t gradualWakeBuzzDelay;
      TimerHandle_t gradualWakeBuzzEnd;
    };
  }
}
