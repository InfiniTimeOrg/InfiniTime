#include "components/alert/AlertController.h"

using namespace Pinetime::Controllers;
using namespace Pinetime::Applications::Display;

AlertController::AlertController(MotorController& motorController) : motorController {motorController} {
}

// return true if a change has happened
bool AlertController::Update() {
  if (motorController.IsVibrating())
    return false;

  bool ok = true;

  if (phoneCallIsActive) {
    ok = motorController.StartRinging(55);
  } else if (timerIsActive) {
    ok = motorController.StartRinging(50);
  } else if (alarmIsActive) {
    ok = motorController.StartRinging(45);
  } else if (notificationIsActive) {
    ok = motorController.SingleVibration(40);
  }

  return ok;
}

Messages AlertController::DisplayMessage() const {
  if (timerIsActive) {
    return Messages::TimerDone;
  } else if (alarmIsActive) {
    return Messages::AlarmTriggered;
  }

  return Messages::NewNotification;
}

void AlertController::ActivatePhoneCall() {
  phoneCallIsActive = true;
};

void AlertController::ActivateTimer() {
  timerIsActive = true;
};

void AlertController::ActivateAlarm() {
  alarmIsActive = true;
};

void AlertController::ActivateNotification() {
  notificationIsActive = true;
};

void AlertController::DeactivatePhoneCall() {
  if (phoneCallIsActive) {
    motorController.StopRinging();
  }
  phoneCallIsActive = false;
};

void AlertController::DeactivateTimer() {
  if (!phoneCallIsActive && timerIsActive) {
    motorController.StopRinging();
  }
  timerIsActive = false;
};

void AlertController::DeactivateAlarm() {
  if (!phoneCallIsActive && !timerIsActive && alarmIsActive) {
    motorController.StopRinging();
  }
  alarmIsActive = false;
};
