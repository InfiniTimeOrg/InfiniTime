#include "components/motor/MotorController.h"
#include <hal/nrf_gpio.h>
#include "systemtask/SystemTask.h"
#include "drivers/PinMap.h"

using namespace Pinetime::Controllers;

void MotorController::Init() {
  nrf_gpio_cfg_output(PinMap::Motor);
  nrf_gpio_pin_set(PinMap::Motor);

  shortVib = xTimerCreate("shortVib", 1, pdFALSE, this, Vibrate);
  longVib = xTimerCreate("longVib", pdMS_TO_TICKS(1000), pdTRUE, this, Ring);
  step = 0;
}

void MotorController::Ring(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(50);
}

void MotorController::RunForDuration(uint8_t motorDuration) {
  StopTune();
  ScheduleVibrateTimer(motorDuration, true);
}

void MotorController::RunTest() {
  StopTune();
  ScheduleVibrateTimer(step, true);
}

void MotorController::StartRinging() {
  RunForDuration(50);
  xTimerStart(longVib, 0);
}

void MotorController::StopRinging() {
  xTimerStop(longVib, 0);
  nrf_gpio_pin_set(PinMap::Motor);
}

void MotorController::StopTune() {
  step =255;
}

void MotorController::StartTune() {
  step = 0;
}

void MotorController::VibrateTune() {
  StartTune();
  ScheduleVibrateTimer(250, true);
  //RunForDuration(5);
}


void MotorController::ScheduleVibrateTimer(uint8_t motorDuration, bool vibrate) {
  if (vibrate) {
    nrf_gpio_pin_clear(PinMap::Motor);
  } else {
    nrf_gpio_pin_set(PinMap::Motor);
  }
  xTimerChangePeriod(shortVib, pdMS_TO_TICKS(motorDuration), 0);
  xTimerStart(shortVib, 0);
}

void MotorController::StopMotor(TimerHandle_t xTimer) {
  nrf_gpio_pin_set(PinMap::Motor);
}

void MotorController::Vibrate(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));

  if (motorController->step < 13){
    motorController->step++;
    bool vibrate = motorController->step % 2;
    motorController->ScheduleVibrateTimer(50, vibrate);
  } else {
    nrf_gpio_pin_set(PinMap::Motor);
  }
}
