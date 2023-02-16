#include "components/motor/MotorController.h"
#include <hal/nrf_gpio.h>
#include "systemtask/SystemTask.h"
#include "drivers/PinMap.h"

using namespace Pinetime::Controllers;

namespace {
  TimerHandle_t vibTimer;

  void PatternStep(TimerHandle_t xTimer) {
    /* Vibration pattern format:
     * {
     *   durationOfVibration,
     *   durationOfPause,
     *   durationOfVibration,
     *   durationOfPause,
     *   ...,
     *   durationOfVibration,
     *   zeroTerminator
     * }
     *
     * Patterns can be any length
     * The pattern must end with a duration of vibration and a terminator.
     */

    static constexpr uint8_t vibrationPattern[] = {30, 150, 30, 150, 30, 0};

    static size_t patternPosition = 0;
    if (vibrationPattern[patternPosition] != 0 && xTimerChangePeriod(vibTimer, vibrationPattern[patternPosition] << 1, 0) == pdPASS &&
        xTimerStart(vibTimer, 0) == pdPASS) {
      if (patternPosition % 2 == 0) {
        nrf_gpio_pin_clear(Pinetime::PinMap::Motor);
      } else {
        nrf_gpio_pin_set(Pinetime::PinMap::Motor);
      }
      patternPosition++;
    } else {
      patternPosition = 0;
      nrf_gpio_pin_set(Pinetime::PinMap::Motor);
      auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
      motorController->PatternFinished();
    }
  }
}

void MotorController::Init() {
  nrf_gpio_cfg_output(PinMap::Motor);
  nrf_gpio_pin_set(PinMap::Motor);

  vibTimer = xTimerCreate("vibration", 1, pdFALSE, this, PatternStep);
  shortVib = xTimerCreate("shortVib", 1, pdFALSE, nullptr, StopMotor);
  longVib = xTimerCreate("longVib", pdMS_TO_TICKS(1000), pdTRUE, this, Ring);
}

void MotorController::PatternFinished() {
  patternPlaying = false;
}

void MotorController::Ring(TimerHandle_t xTimer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(xTimer));
  motorController->RunForDuration(50);
}

void MotorController::RunForDuration(uint8_t motorDuration) {
  if (motorDuration > 0 && xTimerChangePeriod(shortVib, pdMS_TO_TICKS(motorDuration), 0) == pdPASS && xTimerStart(shortVib, 0) == pdPASS) {
    nrf_gpio_pin_clear(PinMap::Motor);
  }
}

bool MotorController::StartPattern() {
  if (!patternPlaying) {
    patternPlaying = true;
    PatternStep(vibTimer);
    return true;
  }
  return false;
}

void MotorController::StartRinging() {
  RunForDuration(50);
  xTimerStart(longVib, 0);
}

void MotorController::StopRinging() {
  xTimerStop(longVib, 0);
  nrf_gpio_pin_set(PinMap::Motor);
}

void MotorController::StopMotor(TimerHandle_t /*xTimer*/) {
  nrf_gpio_pin_set(PinMap::Motor);
}
