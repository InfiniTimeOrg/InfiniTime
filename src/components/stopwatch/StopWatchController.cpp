#include "components/stopwatch/StopWatchController.h"

using namespace Pinetime::Controllers;

namespace {
  TickType_t CalculateDelta(const TickType_t startTime, const TickType_t currentTime) {
    TickType_t delta = 0;
    // Take care of overflow
    if (startTime > currentTime) {
      delta = 0xffffffff - startTime;
      delta += (currentTime + 1);
    } else {
      delta = currentTime - startTime;
    }
    return delta;
  }
}

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
  timeElapsedPreviously += CalculateDelta(startTime, xTaskGetTickCount());
}

void StopWatchController::Clear() {
  currentState = StopWatchStates::Cleared;
  timeElapsedPreviously = 0;

  for (int i = 0; i < LAP_CAPACITY; i++) {
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
  lapHead = lapCount % LAP_CAPACITY;
}

int StopWatchController::GetLapNum() {
  if (lapCount < LAP_CAPACITY)
    return lapCount;
  else
    return LAP_CAPACITY;
}

int StopWatchController::GetLapCount() {
  return lapCount;
}

int Wrap(int index) {
  return ((index % LAP_CAPACITY) + LAP_CAPACITY) % LAP_CAPACITY;
}

LapInfo* StopWatchController::LastLap(int lap) {
  if (lap >= LAP_CAPACITY || lap > lapCount || lapCount == 0) {
    // Return "empty" LapInfo_t
    return &emptyLapInfo;
  }
  // Index backwards
  int index = Wrap(lapHead - lap);
  return &laps[index];
}

// Data / State acess

TickType_t StopWatchController::GetElapsedTime() {
  if (!IsRunning()) {
    return timeElapsedPreviously;
  }
  return timeElapsedPreviously + CalculateDelta(startTime, xTaskGetTickCount());
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
