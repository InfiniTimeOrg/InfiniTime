#pragma once

#include <FreeRTOS.h>
#include <timers.h>

#include <chrono>

namespace Pinetime {
  namespace Controllers {
    class Timer {
    public:
      using ticks = std::chrono::duration<TickType_t, std::ratio<1, configTICK_RATE_HZ>>;
      Timer(void* timerData, TimerCallbackFunction_t timerCallbackFunction);

      void StartTimer(ticks duration);

      void StopTimer();

      ticks GetTimeRemaining();

      bool IsRunning();

    private:
      TimerHandle_t timer;
    };
  }
}
