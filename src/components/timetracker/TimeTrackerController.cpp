#include "components/timetracker/TimeTrackerController.h"
#include <algorithm> // std::fill

using namespace Pinetime::Controllers;

void TimeTrackerController::Demo() {
  printf("0 %i\n", this->totals[0]);
  printf("1 %i\n", this->totals[1]);
  printf("2 %i\n", this->totals[2]);
  printf("3 %i\n", this->totals[3]);
  printf("4 %i\n", this->totals[4]);
  printf("5 %i\n", this->totals[5]);
}

void TimeTrackerController::Reset() {
  std::fill(this->totals, this->totals + 6, 0);
}

void TimeTrackerController::ModeChanged(TimeTrackingMode newMode) {
  TickType_t currTime = xTaskGetTickCount();

  if (TimeTrackingMode::Iddle != this->currMode) {
    // Calculate elapsed time since last
    TickType_t elapsedTime = currTime - this->startTime;
    // Add elapsed to the current mode
    auto totalsIndex = static_cast<int>(this->currMode);
    this->totals[totalsIndex] += elapsedTime;
  }

  this->currMode = newMode;
  this->startTime = currTime;
}

TickType_t TimeTrackerController::TimeInMode(TimeTrackingMode mode) {
  auto totalsIndex = static_cast<int>(mode);
  return this->totals[totalsIndex];
}

TimeTrackingMode TimeTrackerController::CurrMode() {
  return this->currMode;
}