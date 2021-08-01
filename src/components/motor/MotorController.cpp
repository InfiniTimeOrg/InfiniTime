#include "MotorController.h"
#include <hal/nrf_gpio.h>
#include "systemtask/SystemTask.h"
#include "app_timer.h"

APP_TIMER_DEF(shortVibTimer);
APP_TIMER_DEF(longVibTimer);

using namespace Pinetime::Controllers;

MotorController::MotorController(Controllers::Settings& settingsController) : settingsController {settingsController} {
}

void MotorController::Init() {
  nrf_gpio_cfg_output(pinMotor);
  nrf_gpio_pin_set(pinMotor);
  app_timer_init();

  app_timer_create(&shortVibTimer, APP_TIMER_MODE_SINGLE_SHOT, vibrate);
  app_timer_create(&longVibTimer, APP_TIMER_MODE_REPEATED, vibrate);
  isBusy = false;
}

void MotorController::runForDuration(uint8_t motorDuration) {

  if (settingsController.GetVibrationStatus() == Controllers::Settings::Vibration::OFF || isBusy)
    return;

  nrf_gpio_pin_clear(pinMotor);
  /* Start timer for motorDuration miliseconds and timer triggers vibrate() when it finishes*/
  app_timer_start(shortVibTimer, APP_TIMER_TICKS(motorDuration), NULL);
}

void MotorController::startRunning(uint8_t motorDuration) {
  if (settingsController.GetVibrationStatus() == Controllers::Settings::Vibration::OFF || isBusy )
    return;
  //prevent other vibrations while running
  isBusy = true;
  nrf_gpio_pin_clear(pinMotor);
  app_timer_start(longVibTimer, APP_TIMER_TICKS(motorDuration), NULL);
}

void MotorController::stopRunning() {
  
  app_timer_stop(longVibTimer);
  nrf_gpio_pin_set(pinMotor);
  isBusy = false;
}

void MotorController::vibrate(void* p_context) {
  if (nrf_gpio_pin_out_read(pinMotor) == 0) {
    nrf_gpio_pin_set(pinMotor);
  } else {
    nrf_gpio_pin_clear(pinMotor);
  }
}
