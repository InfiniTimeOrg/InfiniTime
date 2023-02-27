#include "components/timer/Timer.h"

using namespace Pinetime::Controllers;

Timer::Timer(void* const timerData, TimerCallbackFunction_t timerCallbackFunction) {
  timer = xTimerCreate("Timer", 1, pdFALSE, timerData, timerCallbackFunction);
}

void Timer::StartTimer(ticks duration) {
  xTimerChangePeriod(timer, duration.count(), 0);
  xTimerStart(timer, 0);
}

Timer::ticks Timer::GetTimeRemaining() {
  if (IsRunning()) {
    return ticks(xTimerGetExpiryTime(timer) - xTaskGetTickCount());
  }
  return ticks(0);
}

void Timer::StopTimer() {
  xTimerStop(timer, 0);
}

bool Timer::IsRunning() {
  return (xTimerIsTimerActive(timer) == pdTRUE);
}
