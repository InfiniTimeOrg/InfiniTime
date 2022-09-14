#include "components/motor/MotorController.h"
#include <hal/nrf_gpio.h>
#include "systemtask/SystemTask.h"
#include "drivers/PinMap.h"

using namespace Pinetime::Controllers;

void MotorController::Init() {
  nrf_gpio_cfg_output(PinMap::Motor);
  nrf_gpio_pin_set(PinMap::Motor);

  shortVib = xTimerCreate("shortVib", 1, pdFALSE, this, StopMotor);
  longVib = xTimerCreate("longVib", pdMS_TO_TICKS(1000), pdTRUE, this, Ring);
}

void MotorController::Ring(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(50);
}

void MotorController::RunForDuration(uint8_t motorDuration) {
  if (motorDuration > 0 && xTimerChangePeriod(shortVib, pdMS_TO_TICKS(motorDuration), 0) == pdPASS && xTimerStart(shortVib, 0) == pdPASS) {
    isShortVibrating = true;
    nrf_gpio_pin_clear(PinMap::Motor);
  }
}

void MotorController::StartRinging(uint8_t Duration) {
  isLongVibrating = true;
  RunForDuration(Duration);
  xTimerStart(longVib, 0);
}

void MotorController::StopRinging() {
  xTimerStop(longVib, 0);
  nrf_gpio_pin_set(PinMap::Motor);
  isLongVibrating = false;
  if (!IsVibrating()) {
    // current vibration pattern is over, so check for next one
    Update();
  }
}

void MotorController::StopMotor(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  nrf_gpio_pin_set(PinMap::Motor);
  motorController->isShortVibrating = false;
  if (!motorController->IsVibrating()) {
    // current vibration pattern is over, so check for next one
    motorController->Update();
  }
}

void MotorController::Update() {
  if (IsVibrating())
    return;

  if (phoneCallIsActive) {
    StartRinging(55);
  } else if (timerIsActive) {
    StartRinging(50);
  } else if (alarmIsActive) {
    StartRinging(45);
  } else if (notificationIsActive) {
    RunForDuration(40);
  }
}

// CAUTION: this one will not trigger a vibration if another one is already running!
void MotorController::SingleVibration(uint8_t Duration) {
  if (IsVibrating()) {
    return;
  }
  RunForDuration(Duration);
}

void MotorController::ActivatePhoneCall() {
  phoneCallIsActive = true;
  Update();
};

void MotorController::ActivateTimer() {
  timerIsActive = true;
  Update();
};

void MotorController::ActivateAlarm() {
  alarmIsActive = true;
  Update();
};

void MotorController::ActivateNotification() {
  notificationIsActive = true;
  Update();
};

void MotorController::DeactivatePhoneCall() {
  phoneCallIsActive = false;
  Update();
};
void MotorController::DeactivateTimer() {
  timerIsActive = false;
  Update();
};

void MotorController::DeactivateAlarm() {
  alarmIsActive = false;
  Update();
};
