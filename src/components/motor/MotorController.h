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

      void SingleVibration(uint8_t Duration);

      bool IsVibrating() {
        return isShortVibrating || isLongVibrating;
      };

      void ActivatePhoneCall();
      void ActivateTimer();
      void ActivateAlarm();
      void ActivateNotification();

      void DeactivatePhoneCall();
      void DeactivateTimer();
      void DeactivateAlarm();

    private:
      void Update();
      void RunForDuration(uint8_t motorDuration);
      void StartRinging();
      void StopRinging();
      
      static void Ring(TimerHandle_t xTimer);
      static void StopMotor(TimerHandle_t xTimer);
      TimerHandle_t shortVib;
      TimerHandle_t longVib;

      bool isShortVibrating = false;
      bool isLongVibrating = false;
      
      bool phoneCallIsActive = false;
      bool timerIsActive = false;
      bool alarmIsActive = false;
      bool notificationIsActive = false;
    };
  }
}
