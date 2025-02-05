#include "components/timer/Timer.h"

using namespace Pinetime::Controllers;

Timer::Timer(void* const timerData, TimerCallbackFunction_t timerCallbackFunction) {
  timerHandle = xTimerCreate("Timer", 1, pdFALSE, timerData, timerCallbackFunction);
}

void Timer::StartTimer(std::chrono::milliseconds duration) {
  timerOverflowIntervals = 0;

  if (duration.count() > maxTimerMS) {
    timerOverflowIntervals = duration.count() / maxTimerMS;
    uint32_t remainingMS = duration.count() % maxTimerMS;
    uint8_t leftoverMinutes = remainingMS / 60 / 1000;
    uint8_t leftoverSeconds = (remainingMS % (60 * 1000)) / 1000;
    if (leftoverMinutes == 0 && leftoverSeconds == 0) {
      leftoverMinutes = 59;
      leftoverSeconds = 60;
      timerOverflowIntervals--;
    }
    xTimerChangePeriod(timerHandle, pdMS_TO_TICKS((leftoverMinutes * 60 * 1000) + (leftoverSeconds * 1000)), 0);
  } else {
    xTimerChangePeriod(timerHandle, pdMS_TO_TICKS(duration.count()), 0);
  }
  xTimerStart(timerHandle, 0);
}

std::chrono::milliseconds Timer::GetTimeRemaining() {
  if (IsRunning()) {
    TickType_t remainingTime = xTimerGetExpiryTime(timerHandle) - xTaskGetTickCount();
    return std::chrono::milliseconds((remainingTime * 1000 / configTICK_RATE_HZ) + (timerOverflowIntervals * maxTimerMS));
  }
  return std::chrono::milliseconds(0);
}

void Timer::StopTimer() {
  xTimerStop(timerHandle, 0);
}

bool Timer::IsRunning() {
  return (xTimerIsTimerActive(timerHandle) == pdTRUE);
}
