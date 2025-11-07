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
  timeElapsedPreviously = GetElapsedTime();
  currentState = StopWatchStates::Paused;
}

void StopWatchController::Clear() {
  currentState = StopWatchStates::Cleared;
  timeElapsedPreviously = 0;

  for (uint8_t i = 0; i < histSize; i++) {
    history[i].number = 0;
    history[i].timeSinceStart = 0;
  }
  maxLapNumber = 0;
}

// Lap

void StopWatchController::AddLapToHistory() {
  TickType_t lapEnd = GetElapsedTime();
  history--;
  history[0].timeSinceStart = lapEnd;
  history[0].number = ++maxLapNumber % lapNumberBoundary;
}

uint16_t StopWatchController::GetMaxLapNumber() {
  return maxLapNumber;
}

std::optional<LapInfo> StopWatchController::GetLapFromHistory(uint8_t index) {
  if (index >= histSize || history[index].number == 0) {
    return {};
  }
  return history[index];
}

// Data / State acess

TickType_t StopWatchController::GetElapsedTime() {
  if (!IsRunning()) {
    return timeElapsedPreviously;
  }
  TickType_t delta = xTaskGetTickCount() - startTime;
  return (timeElapsedPreviously + delta) % elapsedTimeBoundary;
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
