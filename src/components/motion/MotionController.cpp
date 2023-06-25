#include "components/motion/MotionController.h"

#include <task.h>

#include "utility/Math.h"

using namespace Pinetime::Controllers;

namespace {
  constexpr inline int32_t Clamp(int32_t val, int32_t min, int32_t max) {
    return val < min ? min : (val > max ? max : val);
  }

  // only returns meaningful values if inputs are acceleration due to gravity
  int16_t DegreesRolled(int16_t y, int16_t z, int16_t prevY, int16_t prevZ) {
    int16_t prevYAngle = Pinetime::Utility::Asin(Clamp(prevY * 32, -32767, 32767));
    int16_t yAngle = Pinetime::Utility::Asin(Clamp(y * 32, -32767, 32767));

    if (z < 0 && prevZ < 0) {
      return yAngle - prevYAngle;
    }
    if (prevZ < 0) {
      if (y < 0) {
        return -prevYAngle - yAngle - 180;
      }
      return -prevYAngle - yAngle + 180;
    }
    if (z < 0) {
      if (y < 0) {
        return prevYAngle + yAngle + 180;
      }
      return prevYAngle + yAngle - 180;
    }
    return prevYAngle - yAngle;
  }
}

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

  stats = GetAccelStats();

  int32_t deltaSteps = nbSteps - this->nbSteps;
  if (deltaSteps > 0) {
    currentTripSteps += deltaSteps;
  }
  this->nbSteps = nbSteps;
}

MotionController::AccelStats MotionController::GetAccelStats() const {
  AccelStats stats;

  for (uint8_t i = 0; i < AccelStats::numHistory; i++) {
    stats.yMean += yHistory[histSize - i];
    stats.zMean += zHistory[histSize - i];
    stats.prevYMean += yHistory[1 + i];
    stats.prevZMean += zHistory[1 + i];
  }
  stats.yMean /= AccelStats::numHistory;
  stats.zMean /= AccelStats::numHistory;
  stats.prevYMean /= AccelStats::numHistory;
  stats.prevZMean /= AccelStats::numHistory;

  for (uint8_t i = 0; i < AccelStats::numHistory; i++) {
    stats.yVariance += (yHistory[histSize - i] - stats.yMean) * (yHistory[histSize - i] - stats.yMean);
    stats.zVariance += (zHistory[histSize - i] - stats.zMean) * (zHistory[histSize - i] - stats.zMean);
  }
  stats.yVariance /= AccelStats::numHistory;
  stats.zVariance /= AccelStats::numHistory;

  return stats;
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
