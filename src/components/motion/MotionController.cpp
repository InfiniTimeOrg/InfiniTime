#include "MotionController.h"
#include "os/os_cputime.h"

using namespace Pinetime::Controllers;

void MotionController::Update(int16_t x, int16_t y, int16_t z, uint32_t nbSteps) {
  this->x = x;
  this->y = y;
  this->z = z;
  this->nbSteps = nbSteps;
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

bool MotionController::Should_ShakeWake() {
  bool wake = false;
  auto diff = xTaskGetTickCount() - lastShakeTime;
  lastShakeTime = xTaskGetTickCount();
  int32_t speed = std::abs(y + z - lastYForShake - lastZForShake) / diff * 10;
  if (speed > 150) { // TODO move threshold to a setting
    wake = true;
  }
  lastXForShake = x;
  lastYForShake = y;
  lastZForShake = z;
  return wake;
}

void MotionController::IsSensorOk(bool isOk) {
  isSensorOk = isOk;
}
void MotionController::Init(Pinetime::Drivers::Bma421::DeviceTypes types) {
  switch(types){
    case Drivers::Bma421::DeviceTypes::BMA421: this->deviceType = DeviceTypes::BMA421; break;
    case Drivers::Bma421::DeviceTypes::BMA425: this->deviceType = DeviceTypes::BMA425; break;
    default: this->deviceType = DeviceTypes::Unknown; break;
  }
}
