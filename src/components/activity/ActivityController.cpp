#include "components/activity/ActivityController.h"

using namespace Pinetime::Controllers;

void ActivityController::UpdateSteps(uint32_t nbSteps, uint8_t minutes) {
  uint8_t hourSegment = minutes * (stepHistory.Size() - 2) / 60;
  if (hourSegment != prevHourSegment && prevHourSegment != UINT8_MAX) {
    stepHistory++;
    canNotify = true;
  }
  prevHourSegment = hourSegment;
  stepHistory[0] = nbSteps;

  if (stepHistory[0] < stepHistory[stepHistory.Size() - 1]) {
    for (uint8_t i = 1; i < stepHistory.Size(); i++) {
      stepHistory[i] = 0;
    }
  }
}

bool ActivityController::ShouldNotify(uint16_t thresh) {
  if (canNotify && stepHistory[stepHistory.Size() - 1] - stepHistory[1] < thresh) {
    canNotify = false;
    return true;
  }
  return false;
}
