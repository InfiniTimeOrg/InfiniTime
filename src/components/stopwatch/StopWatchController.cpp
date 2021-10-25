#include "StopWatchController.h"
#include <cstdlib>
#include <cstring>

using namespace Pinetime::Controllers;

namespace {
  TickType_t calculateDelta(const TickType_t startTime, const TickType_t currentTime) {
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

StopWatch::StopWatch() {
  clear();
}

// State Change

void StopWatch::start(TickType_t start) {
  currentState = StopWatchStates::Running;
  startTime = start;
}

void StopWatch::pause(TickType_t end) {
  currentState = StopWatchStates::Paused;
  timeElapsedPreviously += calculateDelta(startTime, end);
}

void StopWatch::clear() {
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

void StopWatch::pushLap(TickType_t lapEnd) {
  laps[lapHead].time = lapEnd;
  laps[lapHead].count = lapCount + 1;
  lapCount += 1;
  lapHead = lapCount % LAP_CAPACITY;
}

uint32_t StopWatch::getLapNum() {
  if (lapCount < LAP_CAPACITY)
    return lapCount;
  else
    return LAP_CAPACITY;
}

uint32_t StopWatch::getLapCount() {
  return lapCount;
}

LapInfo_t *StopWatch::lastLap(uint32_t lap) {
  if (lap >= LAP_CAPACITY || lap >= lapCount || lapCount == 0) {
    // Return "empty" LapInfo_t
    return &emptyLapInfo;
  }
  // Index backwards
  uint32_t index = ((lapHead + LAP_CAPACITY) - lap) % LAP_CAPACITY;
  return &laps[index];
}

// Data acess

uint32_t StopWatch::getStart() {
  return startTime;
}

uint32_t StopWatch::getElapsedPreviously() {
  return timeElapsedPreviously;
}

bool StopWatch::isRunning() {
  return currentState == StopWatchStates::Running;
}

bool StopWatch::isCleared() {
  return currentState == StopWatchStates::Cleared;
}

bool StopWatch::isPaused() {
  return currentState == StopWatchStates::Paused;
}
