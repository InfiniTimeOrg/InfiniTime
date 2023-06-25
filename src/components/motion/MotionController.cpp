#include "components/motion/MotionController.h"

#include <task.h>

using namespace Pinetime::Controllers;

void MotionController::Update(int16_t x, int16_t y, int16_t z, uint32_t nbSteps) {
  if (this->nbSteps != nbSteps && service != nullptr) {
    service->OnNewStepCountValue(nbSteps);
  }

  if (service != nullptr && (this->x != x || yHistory[0] != y || zHistory[0] != z)) {
    service->OnNewMotionValues(x, y, z);
  }

  lastTime = time;
  time = xTaskGetTickCount();

  lastX = this->x;
  this->x = x;
  yHistory++;
  yHistory[0] = y;
  zHistory++;
  zHistory[0] = z;

  int32_t deltaSteps = nbSteps - this->nbSteps;
  if (deltaSteps > 0) {
    currentTripSteps += deltaSteps;
  }
  this->nbSteps = nbSteps;
}

bool MotionController::ShouldRaiseWake(bool isSleeping) {
  if ((x + 335) <= 670 && zHistory[0] < 0) {
    if (!isSleeping) {
      if (yHistory[0] <= 0) {
        return false;
      }
      lastYForRaiseWake = 0;
      return false;
    }

    if (yHistory[0] >= 0) {
      lastYForRaiseWake = 0;
      return false;
    }
    if (yHistory[0] + 230 < lastYForRaiseWake) {
      lastYForRaiseWake = yHistory[0];
      return true;
    }
  }
  return false;
}

bool MotionController::ShouldShakeWake(uint16_t thresh) {
  /* Currently Polling at 10hz, If this ever goes faster scalar and EMA might need adjusting */
  int32_t speed =
    std::abs(zHistory[0] - zHistory[histSize - 1] + (yHistory[0] - yHistory[histSize - 1]) / 2 + (x - lastX) / 4) * 100 / (time - lastTime);
  // (.2 * speed) + ((1 - .2) * accumulatedSpeed);
  accumulatedSpeed = speed / 5 + accumulatedSpeed * 4 / 5;

  return accumulatedSpeed > thresh;
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
