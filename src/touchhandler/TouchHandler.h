#pragma once
#include "drivers/Cst816s.h"
#include "systemtask/SystemTask.h"
#include <FreeRTOS.h>
#include <task.h>

namespace Pinetime {
  namespace Components {
    class LittleVgl;
  }
  namespace Drivers {
    class Cst816S;
  }
  namespace System {
    class SystemTask;
  }
  namespace Controllers {
    class TouchHandler {
      public:
        explicit TouchHandler(Drivers::Cst816S&, Components::LittleVgl&);
        void CancelTap();
        void Register(Pinetime::System::SystemTask* systemTask);
        void Start();
        void WakeUp();
        uint8_t GetX() const {
          return x;
        }
        uint8_t GetY() const {
          return y;
        }
        Drivers::Cst816S::Gestures GestureGet();
      private:
        static void Process(void* instance);
        void Work();
        Pinetime::System::SystemTask* systemTask = nullptr;
        TaskHandle_t taskHandle;
        Pinetime::Drivers::Cst816S& touchPanel;
        Pinetime::Components::LittleVgl& lvgl;
        Pinetime::Drivers::Cst816S::Gestures gesture;
        bool isCancelled = false;
        uint8_t x, y;
    };
  }
}
