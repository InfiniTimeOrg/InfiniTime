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

/*
Some explaining for my future self:

The "shortVib" variable is a timer. We activate/deactivate the motor for
stretches of time based on this timer. If you want the watch to vibrates, you
call:
`nrf_gpio_pin_clear(PinMap::Motor)` (turns vibrator on)
then `xTimerStart(shortVib, 0)` (starts the timer).

When initialized, the shortVib variable had a `Vibrate` function attached to it.
This function includes a call to `nrf_gpio_pin_set(PinMap::Motor)`, which turns
the vibrator off. The timer calls this function when it completes, and thus
the vibrator is only on for the duration of the timer. Vibrate includes other
stuff, but for the standard `RunForDuration` above it skips all that and just
ends the vibration.

However, in this function, I activate the other segments of the `Vibrate`
function. I basically have the timer end/start the vibrator, then start another
timer, then end/start the vibrator, etc., giving it a staggered on/off vibration
pattern. It does this ~6 times in rapid succession. I start it off with a longer
vibration, so the effect is a long vibration followed by several short
vibrations. Once the number of repeats exceeds a certain amount, it just kills
the vibrator.

The repeats are tracked with the `step` variable. When doing a customized
vibration I set this to 0, then increment it with each pass until it exceeds
a given amount. For `RunForDuration` I just initialize `step` to a large number
so it defaults to just ending the vibration.
*/
void MotorController::VibrateTune() {
  StartTune();
  ScheduleVibrateTimer(250, true);
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
