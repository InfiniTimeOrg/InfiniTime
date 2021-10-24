#include "StopWatchController.h"
#include <cstdlib>
#include <cstring>

using namespace Pinetime::Controllers;

StopWatch::StopWatch() {}

// StopWatch::init() {}

void StopWatch::start(uint32_t start) {
  currentState = StopWatchStates::Running;
  startTime = start;
}

// void StopWatch::lap(uint32_t lapEnd);

void StopWatch::pause(uint32_t end) {
  currentState = StopWatchStates::Paused;
  timeElapsedPreviously += end - startTime;
}

void StopWatch::clear() {
  currentState = StopWatchStates::Cleared;
  timeElapsedPreviously = 0;
}

uint32_t StopWatch::getStart() {
  return startTime;
}

uint32_t StopWatch::getElapsedPreviously() {
  return timeElapsedPreviously;
}

bool StopWatch::isRunning() {
  return currentState == StopWatchStates::Running;
}

bool StopWatch::isClear() {
  return currentState == StopWatchStates::Cleared;
}

bool StopWatch::isPaused() {
  return currentState == StopWatchStates::Paused;
}
