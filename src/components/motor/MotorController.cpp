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

void MotorController::Init() {
  nrf_gpio_cfg_output(pinMotor);
  nrf_gpio_pin_set(pinMotor);
  app_timer_init();

  app_timer_create(&shortVibTimer, APP_TIMER_MODE_SINGLE_SHOT, Vibrate);
  app_timer_create(&longVibTimer, APP_TIMER_MODE_REPEATED, Ring);
}

void MotorController::Ring(void* p_context) {
  auto* motorController = static_cast<MotorController*>(p_context);
  motorController->VibrateTune(TuneType::RING);
}

void MotorController::RunForDuration(uint8_t motorDuration) {
  if (settingsController.GetVibrationStatus() == Controllers::Settings::Vibration::OFF) {
    return;
  }
  StopTune();
  ScheduleVibrateTimer(motorDuration, true);
}


void MotorController::StopTune() {
  step = 255;
}

void MotorController::ScheduleTune(TuneType tune) {
  step = 0;
  runningTune = tune;
}

/**
* schedule next vibrate timer tick with or without vibration
*/

void MotorController::ScheduleVibrateTimer(uint8_t motorDuration, bool vibrate) {
  if (vibrate) {  
    nrf_gpio_pin_clear(pinMotor);
  } else {
    nrf_gpio_pin_set(pinMotor);
  }
  app_timer_start(shortVibTimer, APP_TIMER_TICKS(motorDuration), this);
}

void MotorController::VibrateTune(TuneType tune) {
  if (settingsController.GetVibrationStatus() == Controllers::Settings::Vibration::OFF)
    return;
  ScheduleTune(tune);
  Vibrate(this);
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
  auto* motorC = static_cast<MotorController*>(p_context);
  auto* runningTune = &tunes[motorC->runningTune];
  
  nrf_gpio_pin_set(pinMotor); //turn off vibration

  //scedule next tune tick
  if (motorC->step < 8 && motorC->step < runningTune->length) {
    bool vibrate = ((1 << motorC->step) & runningTune->tune) > 0;
    motorC->step++;

    motorC->ScheduleVibrateTimer(runningTune->tempo, vibrate);
  } 
}
