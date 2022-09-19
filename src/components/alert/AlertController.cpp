#include "components/motor/AlertController.h"

using namespace Pinetime::Controllers;

AlertController::AlertController(MotorController& motorController)
  : motorController {motorController} {
}

void AlertController::Update() {
  // this can be a bug. A SingleVibration can break the Update-chain even if there are still things active.
  // maybe we have to trigger Update through the system task in each iteration 
  if (motorController.IsVibrating())
    return;

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
  
  // handle not ok
}

void AlertController::ActivatePhoneCall() {
  phoneCallIsActive = true;
  Update();
};

void AlertController::ActivateTimer() {
  timerIsActive = true;
  Update();
};

void AlertController::ActivateAlarm() {
  alarmIsActive = true;
  Update();
};

void AlertController::ActivateNotification() {
  notificationIsActive = true;
  Update();
};

void AlertController::DeactivatePhoneCall() {
  if (phoneCallIsActive) {
    motorController.StopRinging();
  }
  phoneCallIsActive = false;
  Update();
};
void AlertController::DeactivateTimer() {
  if (!phoneCallIsActive && timerIsActive) {
    motorController.StopRinging();
  }
  timerIsActive = false;
  Update();
};

void AlertController::DeactivateAlarm() {
  if (!phoneCallIsActive && !timerIsActive && alarmIsActive) {
    motorController.StopRinging();
  }
  alarmIsActive = false;
  Update();
};
