#include "MotorController.h"
#include <hal/nrf_gpio.h>
#include "systemtask/SystemTask.h"
#include "app_timer.h"
#include "drivers/PinMap.h"

APP_TIMER_DEF(shortVibTimer);
APP_TIMER_DEF(longVibTimer);

using namespace Pinetime::Controllers;

MotorController::MotorController(Controllers::Settings& settingsController) : settingsController {settingsController} {
}

void MotorController::Init() {
  nrf_gpio_cfg_output(PinMap::Motor);
  nrf_gpio_pin_set(PinMap::Motor);
  app_timer_init();

  app_timer_create(&shortVibTimer, APP_TIMER_MODE_SINGLE_SHOT, StopMotor);
  app_timer_create(&longVibTimer, APP_TIMER_MODE_REPEATED, Ring);
}

void MotorController::Ring(void* p_context) {
  auto* motorController = static_cast<MotorController*>(p_context);
  motorController->RunForDuration(50);
}

void MotorController::RunForDuration(uint8_t motorDuration) {
  if (settingsController.GetVibrationStatus() == Controllers::Settings::Vibration::OFF) {
    return;
  }

  nrf_gpio_pin_clear(PinMap::Motor);
  app_timer_start(shortVibTimer, APP_TIMER_TICKS(motorDuration), nullptr);
}

void MotorController::StartRinging() {
  if (settingsController.GetVibrationStatus() == Controllers::Settings::Vibration::OFF) {
    return;
  }
  Ring(this);
  app_timer_start(longVibTimer, APP_TIMER_TICKS(1000), this);
}

void MotorController::StopRinging() {
  app_timer_stop(longVibTimer);
  nrf_gpio_pin_set(PinMap::Motor);
}

void MotorController::StopMotor(void* p_context) {
  nrf_gpio_pin_set(PinMap::Motor);
}
