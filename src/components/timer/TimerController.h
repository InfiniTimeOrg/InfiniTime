#pragma once

#include <FreeRTOS.h>
#include <timers.h>

#include <chrono>

namespace Pinetime {
  namespace System {
    class SystemTask;
  }

  namespace Controllers {

    class TimerController {
    public:
      TimerController() = default;

      void Init(System::SystemTask* systemTask);

      void StartTimer(std::chrono::milliseconds duration);

      void StopTimer();

      std::chrono::milliseconds GetTimeRemaining();

      bool IsRunning();

      void OnTimerEnd();

    private:
      System::SystemTask* systemTask = nullptr;
      TimerHandle_t timer;
    };
  }
}
