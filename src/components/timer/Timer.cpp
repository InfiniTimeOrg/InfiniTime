#include "components/timer/Timer.h"

using namespace Pinetime::Controllers;

Timer::Timer(void* const timerData, TimerCallbackFunction_t timerCallbackFunction) {
  timer = xTimerCreate("Timer", 1, pdFALSE, timerData, timerCallbackFunction);
}

void Timer::StartTimer(std::chrono::milliseconds duration) {
  xTimerChangePeriod(timer, pdMS_TO_TICKS(duration.count()), 0);
  xTimerStart(timer, 0);
}

std::chrono::milliseconds Timer::GetTimeRemaining() {
  if (IsRunning()) {
    TickType_t remainingTime = xTimerGetExpiryTime(timer) - xTaskGetTickCount();
    return std::chrono::milliseconds(remainingTime * 1000 / configTICK_RATE_HZ);
  }
  return std::chrono::milliseconds(0);
}

void Timer::StopTimer() {
  xTimerStop(timer, 0);
}

bool Timer::IsRunning() {
  return (xTimerIsTimerActive(timer) == pdTRUE);
}
