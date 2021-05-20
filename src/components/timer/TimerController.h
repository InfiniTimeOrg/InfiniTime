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
      TimerController(Pinetime::System::SystemTask& systemTask);
      
      void Init();
      
      void StartTimer(uint32_t duration);
      
      void StopTimer();
      
      uint32_t GetTimeRemaining();
      
      bool IsRunning();
    
    private:
      System::SystemTask& systemTask;
      
      static void timerEnd(void* p_context);
  
      TickType_t endTicks;
      bool timerRunning = false;
    };
  }
}