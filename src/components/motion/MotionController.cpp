#include "components/motion/MotionController.h"
#include "os/os_cputime.h"

using namespace Pinetime::Controllers;

MotionController::MotionController(Pinetime::Drivers::Bma421& motionSensor) : motionSensor {motionSensor} {
}

void MotionController::Update() {
  isSensorOk = motionSensor.IsOk();

  if (stepCounterMustBeReset) {
    motionSensor.ResetStepCounter();
    stepCounterMustBeReset = false;
  }

  auto motionValues = motionSensor.Process();

  if (nbSteps != motionValues.steps && service != nullptr) {
    service->OnNewStepCountValue(nbSteps);
  }

  if (service != nullptr && (x != motionValues.x || y != motionValues.y || z != motionValues.z)) {
    service->OnNewMotionValues(motionValues.x, motionValues.y, motionValues.z);
  }

  x = motionValues.x;
  y = motionValues.y;
  z = motionValues.z;
  int32_t deltaSteps = motionValues.steps - nbSteps;
  if (deltaSteps > 0) {
    currentTripSteps += deltaSteps;
  }
  nbSteps = motionValues.steps;
}

bool MotionController::Should_RaiseWake(bool isSleeping) {
  if ((x + 335) <= 670 && z < 0) {
    if (not isSleeping) {
      if (y <= 0) {
        return false;
      } else {
        lastYForWakeUp = 0;
        return false;
      }
    }

    if (y >= 0) {
      lastYForWakeUp = 0;
      return false;
    }
    if (y + 230 < lastYForWakeUp) {
      lastYForWakeUp = y;
      return true;
    }
  }
  return false;
}

bool MotionController::Should_ShakeWake(uint16_t thresh) {
  bool wake = false;
  auto diff = xTaskGetTickCount() - lastShakeTime;
  lastShakeTime = xTaskGetTickCount();
  /* Currently Polling at 10hz, If this ever goes faster scalar and EMA might need adjusting */
  int32_t speed = std::abs(z + (y / 2) + (x / 4) - lastYForShake - lastZForShake) / diff * 100;
  //(.2 * speed) + ((1 - .2) * accumulatedspeed);
  // implemented without floats as .25Alpha
  accumulatedspeed = (speed / 5) + ((accumulatedspeed / 5) * 4);

  if (accumulatedspeed > thresh) {
    wake = true;
  }
  lastXForShake = x / 4;
  lastYForShake = y / 2;
  lastZForShake = z;
  return wake;
}
int32_t MotionController::currentShakeSpeed() {
  return accumulatedspeed;
}

void MotionController::Init(Pinetime::Drivers::Bma421::DeviceTypes types) {
  switch (types) {
    case Drivers::Bma421::DeviceTypes::BMA421:
      this->deviceType = DeviceTypes::BMA421;
      break;
    case Drivers::Bma421::DeviceTypes::BMA425:
      this->deviceType = DeviceTypes::BMA425;
      break;
    default:
      this->deviceType = DeviceTypes::Unknown;
      break;
  }
}
void MotionController::SetService(Pinetime::Controllers::MotionService* service) {
  this->service = service;
}
