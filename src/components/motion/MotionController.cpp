#include "components/motion/MotionController.h"

#include <task.h>

using namespace Pinetime::Controllers;

void MotionController::Update(int16_t x, int16_t y, int16_t z, uint32_t nbSteps) {
  if (this->nbSteps != nbSteps && service != nullptr) {
    service->OnNewStepCountValue(nbSteps);
  }

  if (service != nullptr && (this->x != x || this->y != y || this->z != z)) {
    service->OnNewMotionValues(x, y, z);
  }

  lastTime = time;
  time = xTaskGetTickCount();

  lastX = this->x;
  this->x = x;
  lastY = this->y;
  this->y = y;
  lastZ = this->z;
  this->z = z;

  int32_t deltaSteps = nbSteps - this->nbSteps;
  if (deltaSteps > 0) {
    currentTripSteps += deltaSteps;
  }
  this->nbSteps = nbSteps;
}

bool MotionController::ShouldRaiseWake(bool isSleeping) {
  if ((x + 335) <= 670 && z < 0) {
    if (!isSleeping) {
      if (y <= 0) {
        return false;
      }
      lastYForRaiseWake = 0;
      return false;
    }

    if (y >= 0) {
      lastYForRaiseWake = 0;
      return false;
    }
    if (y + 230 < lastYForRaiseWake) {
      lastYForRaiseWake = y;
      return true;
    }
  }
  return false;
}

bool MotionController::ShouldShakeWake(uint16_t thresh) {
  /* Currently Polling at 10hz, If this ever goes faster scalar and EMA might need adjusting */
  int32_t speed = std::abs(z - lastZ + (y / 2) - (lastY / 2) + (x / 4) - (lastX / 4)) / (time - lastTime) * 100;
  //(.2 * speed) + ((1 - .2) * accumulatedSpeed);
  // implemented without floats as .25Alpha
  accumulatedSpeed = (speed / 5) + ((accumulatedSpeed / 5) * 4);

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
