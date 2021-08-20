#include "MotorController.h"
#include <hal/nrf_gpio.h>
#include "systemtask/SystemTask.h"
#include "app_timer.h"

APP_TIMER_DEF(shortVibTimer);
APP_TIMER_DEF(longVibTimer);

using namespace Pinetime::Controllers;

constexpr MotorController::Tune MotorController::tunes[];


MotorController::MotorController(Controllers::Settings& settingsController) : settingsController {settingsController} {
}



uint8_t MotorController::step = 0;
MotorController::TuneType MotorController::runningTune = MotorController::TuneType::STOP; 



void MotorController::Init() {
  nrf_gpio_cfg_output(pinMotor);
  nrf_gpio_pin_set(pinMotor);
  app_timer_init();

  app_timer_create(&shortVibTimer, APP_TIMER_MODE_SINGLE_SHOT, Vibrate);
  app_timer_create(&longVibTimer, APP_TIMER_MODE_REPEATED, Ring);
}

void MotorController::Ring(void* p_context) {
  auto* motorController = static_cast<MotorController*>(p_context);
  motorController->RunForDuration(50);
}

void MotorController::RunForDuration(uint8_t motorDuration) {
  nrf_gpio_pin_set(pinMotor);
  if (settingsController.GetVibrationStatus() == Controllers::Settings::Vibration::OFF) {
    return;
  }
  step = 0;
  runningTune = TuneType::STOP;
  nrf_gpio_pin_clear(pinMotor);
  app_timer_start(shortVibTimer, APP_TIMER_TICKS(motorDuration), nullptr);

}

void MotorController::VibrateTune(TuneType tune) {
  nrf_gpio_pin_set(pinMotor);
  if (settingsController.GetVibrationStatus() == Controllers::Settings::Vibration::OFF)
    return;
  step = 0;
  runningTune = tune;
  Vibrate(nullptr);
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
  nrf_gpio_pin_set(pinMotor);
}


void MotorController::Vibrate(void* p_context) {
  
  if (step >= tunes[runningTune].length || step >= 8) { //end of tune turn off vibration
    nrf_gpio_pin_set(pinMotor);
    return; 
  }  
 
  if (((1 << step) & tunes[runningTune].tune) > 0) {
    nrf_gpio_pin_clear(pinMotor);
  } else {
    nrf_gpio_pin_set(pinMotor);
  }

  ++step;     
  /* Start timer for the next cycle */
  app_timer_start(shortVibTimer, APP_TIMER_TICKS(tunes[runningTune].tempo), NULL);
}

