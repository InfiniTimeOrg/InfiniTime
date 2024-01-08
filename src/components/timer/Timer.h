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

      void SetExpiredTime();

      void ResetExpiredTime();

    private:
      TimerHandle_t timer;
      TickType_t expired = 0;
    };
  }
}
