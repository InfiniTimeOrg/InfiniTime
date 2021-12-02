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

int StopWatch::getLapNum() {
  if (lapCount < LAP_CAPACITY)
    return lapCount;
  else
    return LAP_CAPACITY;
}

int StopWatch::getLapCount() {
  return lapCount;
}

int wrap(int index) {
  return ((index % LAP_CAPACITY) + LAP_CAPACITY) % LAP_CAPACITY;
}

LapInfo_t* StopWatch::lastLap(int lap) {
  if (lap >= LAP_CAPACITY || lap > lapCount || lapCount == 0) {
    // Return "empty" LapInfo_t
    return &emptyLapInfo;
  }
  // Index backwards
  int index = wrap(lapHead - lap);
  return &laps[index];
}

// Data acess

TickType_t StopWatch::getStart() {
  return startTime;
}

TickType_t StopWatch::getElapsedPreviously() {
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
