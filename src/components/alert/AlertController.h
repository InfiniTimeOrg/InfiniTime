#pragma once

#include "components/motor/MotorController.h"
#include "displayapp/Messages.h"

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

      bool Update();
      Pinetime::Applications::Display::Messages DisplayMessage() const;
    private:
      bool phoneCallIsActive = false;
      bool timerIsActive = false;
      bool alarmIsActive = false;
      bool notificationIsActive = false;

      MotorController& motorController;
    };
  }
}
