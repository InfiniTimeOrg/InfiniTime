#include "components/timer/Timer.h"

using namespace Pinetime::Controllers;

Timer::Timer(void* const timerData, TimerCallbackFunction_t timerCallbackFunction) {
  pausedAtTimer = std::chrono::seconds(0);
  state = Stopped;
  timer = xTimerCreate("Timer", 1, pdFALSE, timerData, timerCallbackFunction);
}

std::chrono::milliseconds Timer::GetTimeRemaining() {
  if (IsRunning()) {
    const TickType_t remainingTime = xTimerGetExpiryTime(timer) - xTaskGetTickCount();
    return std::chrono::milliseconds(remainingTime * 1000 / configTICK_RATE_HZ);
  }

  if (state == Paused)
    return pausedAtTimer;

  return std::chrono::milliseconds(0);
}

Timer::TimerState Timer::GetState() {
  return state;
}

void Timer::Start(const std::chrono::milliseconds duration) {
  TimerStart(duration);
  state = Running;
}

void Timer::Stop() {
  TimerStop();
  state = Stopped;
}

void Timer::Pause() {
  pausedAtTimer = GetTimeRemaining();
  TimerStop();
  state = Paused;
}

void Timer::Resume() {
  TimerStart(pausedAtTimer);
  state = Running;
}

bool Timer::IsRunning() {
  return (xTimerIsTimerActive(timer) == pdTRUE);
};

void Timer::TimerStart(const std::chrono::milliseconds duration) {
  xTimerChangePeriod(timer, pdMS_TO_TICKS(duration.count()), 0);
  xTimerStart(timer, 0);
}

void Timer::TimerStop() {
  xTimerStop(timer, 0);
}
