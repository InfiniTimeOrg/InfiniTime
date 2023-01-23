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

bool MotorController::StartRinging(uint8_t Duration) {
  if (IsVibrating()) {
    return false;
  }
  isLongVibrating = true;
  RunForDuration(Duration);
  xTimerStart(longVib, 0);
  return true;
}

void MotorController::StopRinging() {
  xTimerStop(longVib, 0);
  nrf_gpio_pin_set(PinMap::Motor);
  isLongVibrating = false;
}

void MotorController::StopMotor(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  nrf_gpio_pin_set(PinMap::Motor);
  motorController->isShortVibrating = false;
}

// CAUTION: this one will not trigger a vibration if another one is already running!
bool MotorController::SingleVibration(uint8_t Duration) {
  if (IsVibrating()) {
    return false;
  }
  RunForDuration(Duration);
  return true;
}
