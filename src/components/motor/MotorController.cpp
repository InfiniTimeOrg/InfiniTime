#include "components/motor/MotorController.h"
#include <cstdint>
#include <hal/nrf_gpio.h>
#include "components/motion/MotionController.h"
#include "drivers/PinMap.h"

using namespace Pinetime::Controllers;

MotorController::MotorController(Pinetime::Controllers::MotionController& motionController) : motionController {motionController} {
}

void MotorController::Init() {
  nrf_gpio_cfg_output(PinMap::Motor);
  nrf_gpio_pin_set(PinMap::Motor);

  nextEvt = xTimerCreate("vib", 1, pdFALSE, this, NextEvent);
}

void MotorController::NextEvent(TimerHandle_t timer) {
  auto* motorController = static_cast<MotorController*>(pvTimerGetTimerID(timer));

  TickType_t deviation = xTaskGetTickCount() - motorController->nextEvtTime;

  TickType_t duration;

  switch (motorController->state) {
    case State::Buzz:
      SetMotorRunning(false);
      motorController->state = State::Idle;
      return;
    case State::RingOn:
      SetMotorRunning(false);
      if (motorController->remainingBuzzes == 1) {
        motorController->state = State::Idle;
        return;
      }
      if (motorController->remainingBuzzes > 1) {
        motorController->remainingBuzzes--;
      }
      motorController->state = State::RingOff;
      duration = motorController->ringPeriod - motorController->ringOnTicks;
      break;
    case State::RingOff:
      SetMotorRunning(true);
      motorController->state = State::RingOn;
      duration = motorController->ringOnTicks;
      break;
    case State::Idle:
      // should never hit the idle state in the timer callback
      assert(false);
      __builtin_trap();
  }
  motorController->nextEvtTime += duration;
  if (duration > deviation) {
    xTimerChangePeriod(timer, duration - deviation, 0);
  } else {
    xTimerChangePeriod(timer, 1, 0);
  }
}

uint16_t MotorController::CalculateLength(Intensity intensity) {
  // don't think shake speed really has units
  // 200 seems sensible, then clamp the multiplier to range 1-3
  // a fancy curve would probably feel better, but this seems to work well
  float durationMultiplier = motionController.CurrentShakeSpeed() / 200.f;
  durationMultiplier = std::max(std::min(durationMultiplier, 3.f), 1.f);

  float length;

  switch (intensity) {
    case Intensity::Light:
      length = 30.f;
      break;
    case Intensity::Medium:
      length = 50.f;
      break;
    case Intensity::Strong:
      length = 90.f;
      break;
  }
  return length * durationMultiplier;
}

void MotorController::RunEvent(void* motorControllerPtr, uint32_t requestedStateInt) {
  auto* motorController = static_cast<MotorController*>(motorControllerPtr);
  auto requestedState = static_cast<RequestEvt>(requestedStateInt);

  // these aren't always set, but if they're not set they're never used
  // so it's safe to copy unconditionally
  motorController->ringOnTicks = motorController->nextOperation.ringOnTicks;
  motorController->ringPeriod = motorController->nextOperation.ringPeriod;
  motorController->remainingBuzzes = motorController->nextOperation.buzzCount;

  if (requestedState == RequestEvt::RequestRingOff) {
    SetMotorRunning(false);
    motorController->state = State::Idle;
    xTimerStop(motorController->nextEvt, 0);
  } else {
    SetMotorRunning(true);
    motorController->nextEvtTime = xTaskGetTickCount() + motorController->ringOnTicks;
    if (requestedState == RequestEvt::RequestBuzz) {
      motorController->state = State::Buzz;
    } else {
      motorController->state = State::RingOn;
    }
    // changing the period starts the timer
    xTimerChangePeriod(motorController->nextEvt, motorController->ringOnTicks, 0);
  }
}

void MotorController::PushEvent(RequestEvt state) {
  // since the timer callback modifies its own state,
  // we need to run timer state changes in the timer task to avoid races
  xTimerPendFunctionCall(RunEvent, this, static_cast<uint32_t>(state), portMAX_DELAY);
}

void MotorController::Buzz(Intensity intensity) {
  nextOperation.ringOnTicks = CalculateLength(intensity);

  PushEvent(RequestEvt::RequestBuzz);
}

void MotorController::SetMotorRunning(bool active) {
  if (active) {
    nrf_gpio_pin_clear(PinMap::Motor);
  } else {
    nrf_gpio_pin_set(PinMap::Motor);
  }
}

// numBuzzes = 0 rings forever
void MotorController::Ring(Intensity intensity, TickType_t ringPeriod, uint16_t numBuzzes) {
  nextOperation.ringOnTicks = CalculateLength(intensity);
  nextOperation.ringPeriod = ringPeriod;
  nextOperation.buzzCount = numBuzzes;

  PushEvent(RequestEvt::RequestRingOn);
}

void MotorController::StopRinging() {
  PushEvent(RequestEvt::RequestRingOff);
}

bool MotorController::IsRinging() {
  return state == State::RingOn || state == State::RingOff;
}
