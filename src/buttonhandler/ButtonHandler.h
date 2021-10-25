#pragma once

#include "systemtask/SystemTask.h"
#include <FreeRTOS.h>
#include <timers.h>

namespace Pinetime {
  namespace Controllers {
    class ButtonHandler {
    public:
      enum events { Press, Release, Timer };
      void Init(Pinetime::System::SystemTask* systemTask);
      void HandleEvent(events event);

    private:
      Pinetime::System::SystemTask* systemTask = nullptr;
      TickType_t releaseTime = 0;
      TimerHandle_t buttonTimer;
      bool buttonPressed = false;
      enum states { Idle, Pressed, Holding, LongHeld };
      states state = Idle;
    };
  }
}
