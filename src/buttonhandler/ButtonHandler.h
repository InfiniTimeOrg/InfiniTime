#pragma once

#include "systemtask/SystemTask.h"
#include <FreeRTOS.h>
#include <timers.h>

namespace Pinetime {
  namespace Controllers {
    class ButtonHandler {
    public:
      enum class Events : uint8_t { Press, Release, Timer };
      void Init(Pinetime::System::SystemTask* systemTask);
      void HandleEvent(Events event);

    private:
      enum class States : uint8_t { Idle, Pressed, Holding, LongHeld };
      Pinetime::System::SystemTask* systemTask = nullptr;
      TickType_t releaseTime = 0;
      TimerHandle_t buttonTimer;
      bool buttonPressed = false;
      States state = States::Idle;
    };
  }
}
