#pragma once

#include <cstdint>
#include "app_timer.h"
#include "portmacro_cmsis.h"

namespace Pinetime {
  namespace System {
    class SystemTask;
  }
  namespace Controllers {
    
    class TimerController {
    public:
      TimerController() = default;
      
      void Init();
      
      void StartTimer(uint32_t duration);
      
      void StopTimer();
      
      uint32_t GetTimeRemaining();
      
      bool IsRunning();

      void OnTimerEnd();

      void Register(System::SystemTask* systemTask);

    private:
      System::SystemTask* systemTask = nullptr;
      TickType_t endTicks;
      bool timerRunning = false;
    };
  }
}