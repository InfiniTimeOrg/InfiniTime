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
  lapHead = 0;
}

// Lap

void StopWatchController::PushLap() {
  TickType_t lapEnd = GetElapsedTime();
  laps[lapHead].time = lapEnd;
  laps[lapHead].count = lapCount + 1;
  lapCount += 1;
  lapHead = lapCount % lapCapacity;
}

int StopWatchController::GetLapCount() {
  return lapCount;
}

int Wrap(int index) {
  return ((index % lapCapacity) + lapCapacity) % lapCapacity;
}

std::optional<LapInfo> StopWatchController::LastLap(int lap) {
  if (lap >= lapCapacity || lap >= lapCount) {
    return {};
  }
  // Index backwards
  int mostRecentLap = lapHead - 1;
  int index = Wrap(mostRecentLap - lap);
  return laps[index];
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
