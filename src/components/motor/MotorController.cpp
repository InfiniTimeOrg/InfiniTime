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

void MotorController::StartRinging() {
  isLongVibrating = true;
  RunForDuration(50);
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
  nrf_gpio_pin_set(PinMap::Motor);
  isShortVibrating = false;
  if (!IsVibrating()) {
    // current vibration pattern is over, so check for next one
    Update();
  }
}

void MotorController::Update() {
  if (IsVibrating())
    return;

  if (phoneCallIsActive) {
    // ring phone call

  } else if (timerIsActive) {
    // ring timer
  } else if (alarmIsActive) {
    // ring alarm
  } else if (notificationIsActive) {
    // ring notification
  }
}

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
