#pragma once

#include "components/motor/MotorController.h"

namespace Pinetime {
  namespace Controllers {

    class AlertController {
    public:
      AlertController(MotorController& motorController);

      bool IsActive() const {
        return phoneCallIsActive || timerIsActive || alarmIsActive || notificationIsActive;
      }

      void ActivatePhoneCall();
      void ActivateTimer();
      void ActivateAlarm();
      void ActivateNotification();

      void DeactivatePhoneCall();
      void DeactivateTimer();
      void DeactivateAlarm();

    private:
      void Update();

      bool phoneCallIsActive = false;
      bool timerIsActive = false;
      bool alarmIsActive = false;
      bool notificationIsActive = false;

      MotorController& motorController;
    };
  }
}
