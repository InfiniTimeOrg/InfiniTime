#include "components/stopwatch/StopWatchController.h"

using namespace Pinetime::Controllers;

StopWatchController::StopWatchController() {
  Clear();
}

// State Change

void StopWatchController::Start() {
  currentState = StopWatchStates::Running;
  startTime = xTaskGetTickCount();
}

void StopWatchController::Pause() {
  currentState = StopWatchStates::Paused;
  timeElapsedPreviously += xTaskGetTickCount() - startTime;
}

void StopWatchController::Clear() {
  currentState = StopWatchStates::Cleared;
  timeElapsedPreviously = 0;

  for (int i = 0; i < lapCapacity; i++) {
    laps[i].count = 0;
    laps[i].time = 0;
  }
  lapCount = 0;
}

// Lap

void StopWatchController::PushLap() {
  TickType_t lapEnd = GetElapsedTime();
  laps[0].time = lapEnd;
  laps[0].count = ++lapCount;
  laps--;
}

int StopWatchController::GetLapCount() {
  return lapCount;
}

std::optional<LapInfo> StopWatchController::LastLap(int lap) {
  if (lap < 0 || lap >= lapCapacity || laps[lap].count == 0) {
    return {};
  }
  return laps[lap];
}

// Data / State acess

TickType_t StopWatchController::GetElapsedTime() {
  if (!IsRunning()) {
    return timeElapsedPreviously;
  }
  return timeElapsedPreviously + (xTaskGetTickCount() - startTime);
}

bool StopWatchController::IsRunning() {
  return currentState == StopWatchStates::Running;
}

bool StopWatchController::IsCleared() {
  return currentState == StopWatchStates::Cleared;
}

bool StopWatchController::IsPaused() {
  return currentState == StopWatchStates::Paused;
}
