#include "components/motor/MotorController.h"
#include <hal/nrf_gpio.h>
#include "systemtask/SystemTask.h"
#include "app_timer.h"
#include "drivers/PinMap.h"

using namespace Pinetime::Controllers;

void MotorController::Init() {
  nrf_gpio_cfg_output(PinMap::Motor);
  nrf_gpio_pin_set(PinMap::Motor);

  shortVib = xTimerCreate("shortVib", 1, pdFALSE, nullptr, StopMotor);
  longVib = xTimerCreate("longVib", 1, pdTRUE, this, Ring);
}

void MotorController::Ring(TimerHandle_t xTimer) {
  auto motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  xTimerChangePeriod(motorController->longVib, APP_TIMER_TICKS(1000), 0);
  motorController->RunForDuration(50);
}

void MotorController::RunForDuration(uint8_t motorDuration) {
  if (xTimerChangePeriod(shortVib, pdMS_TO_TICKS(motorDuration), 0) == pdPASS && xTimerStart(shortVib, 0) == pdPASS) {
    nrf_gpio_pin_clear(PinMap::Motor);
  }
}

void MotorController::StartRinging() {
  xTimerChangePeriod(longVib, 1, 0);
  xTimerStart(longVib, 0);
}

void MotorController::StopRinging() {
  xTimerStop(longVib, 0);
  nrf_gpio_pin_set(PinMap::Motor);
}

void MotorController::StopMotor(TimerHandle_t xTimer) {
  nrf_gpio_pin_set(PinMap::Motor);
}
