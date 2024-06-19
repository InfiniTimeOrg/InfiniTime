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
  TickType_t remainingTime = 0;
  if (IsRunning()) {
    remainingTime = xTimerGetExpiryTime(timer) - xTaskGetTickCount();
  } else {
    remainingTime = xTaskGetTickCount() - xTimerGetExpiryTime(timer);
  }
  return std::chrono::milliseconds(remainingTime * 1000 / configTICK_RATE_HZ);
}

void Timer::StopTimer() {
  xTimerStop(timer, 0);
}

bool Timer::IsRunning() {
  return (xTimerIsTimerActive(timer) == pdTRUE);
}
