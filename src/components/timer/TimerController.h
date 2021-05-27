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
      struct TimerValue {
        uint8_t minutes;
        uint8_t seconds;
      };
      
      TimerController(Pinetime::System::SystemTask& systemTask);
      
      void Init();
      
      void StartTimer(uint32_t duration);
      
      void StopTimer();
      
      uint32_t GetTimeRemaining();
      
      bool IsRunning();
  
      void selectPreset(int8_t i);
      void setPreset(int8_t i);
  
      TimerValue* getInitialValue();
      const TimerValue& getPreset(uint8_t i) const;
      
    private:
  
      TimerValue presets[3] = {
          {15, 00},
          {30, 00},
          {00, 30}
      };
   
      TimerValue initialValue = {00, 00};
  
      System::SystemTask& systemTask;
      
      static void timerEnd(void* p_context);
  
      TickType_t endTicks;
      bool timerRunning = false;
    };
  }
}