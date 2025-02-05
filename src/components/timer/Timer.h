#pragma once

#include <FreeRTOS.h>
#include <timers.h>

#include <chrono>

namespace Pinetime {
  namespace Controllers {
    class Timer {
    public:
      Timer(void* timerData, TimerCallbackFunction_t timerCallbackFunction);

      void StartTimer(std::chrono::milliseconds duration);

      void StopTimer();

      std::chrono::milliseconds GetTimeRemaining();

      bool IsRunning();

      uint8_t timerOverflowIntervals = 0;

      TimerHandle_t timerHandle;

      const uint32_t maxTimerMS = 3'600'000; // 1 hour
    private:
    };
  }
}
