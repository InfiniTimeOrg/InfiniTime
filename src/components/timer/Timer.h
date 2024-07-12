#pragma once

#include <FreeRTOS.h>
#include <timers.h>

#include <chrono>

namespace Pinetime {
  namespace Controllers {
    class Timer {
    public:
      Timer(void* timerData, TimerCallbackFunction_t timerCallbackFunction);

      void Start(const std::chrono::milliseconds duration);

      void Stop();

      void Pause();

      void Resume();

      std::chrono::milliseconds GetTimeRemaining();

      typedef enum TimerState { Running, Stopped, Paused } TimerState;

      TimerState GetState();

    private:
      TimerHandle_t timer;

      std::chrono::milliseconds pausedAtTimer;

      TimerState state;

      bool IsRunning();
      void TimerStart(const std::chrono::milliseconds duration);
      void TimerStop();
    };
  }
}
