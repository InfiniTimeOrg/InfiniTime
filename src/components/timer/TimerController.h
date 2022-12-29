#pragma once

#include <FreeRTOS.h>
#include <timers.h>

namespace Pinetime {
  namespace System {
    class SystemTask;
  }

  namespace Controllers {

    class TimerController {
    public:
      TimerController() = default;

      void Init(System::SystemTask* systemTask);

      void StartTimer(uint32_t duration);

      void StopTimer();

      uint32_t GetTimeRemaining();

      void OnTimerEnd();
      void StopRinging();

      enum class TimerState { Stopped, Running, Finished };

      TimerState State() const {
        return state;
      }

    private:
      System::SystemTask* systemTask = nullptr;
      TimerHandle_t timer;
      TimerState state = TimerState::Stopped;
      TimerHandle_t stopRingingTimer;
      bool ringing = false;
    };
  }
}
