#include "MotorController.h"
#include <hal/nrf_gpio.h>
#include "systemtask/SystemTask.h"
#include "app_timer.h"

APP_TIMER_DEF(vibTimer);

using namespace Pinetime::Controllers;

MotorController::MotorController(Controllers::Settings& settingsController) : settingsController {settingsController} {
}

void MotorController::Init() {
  nrf_gpio_cfg_output(pinMotor);
  nrf_gpio_pin_set(pinMotor);
  app_timer_create(&vibTimer, APP_TIMER_MODE_SINGLE_SHOT, vibrate);
}

void MotorController::SetDuration(uint8_t motorDuration) {

  if (settingsController.GetVibrationStatus() == Controllers::Settings::Vibration::OFF)
    return;

  nrf_gpio_pin_clear(pinMotor);
  /* Start timer for motorDuration miliseconds and timer triggers vibrate() when it finishes*/
  app_timer_start(vibTimer, APP_TIMER_TICKS(motorDuration), NULL);
}

void MotorController::vibrate(void* p_context) {
  nrf_gpio_pin_set(pinMotor);
}