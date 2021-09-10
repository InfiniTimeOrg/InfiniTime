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
        bool GetNewTouchInfo();
        void UpdateLvglTouchPoint();

        bool IsTouching() const {
          return info.touching;
        }
        uint8_t GetX() const {
          return info.x;
        }
        uint8_t GetY() const {
          return info.y;
        }
        Drivers::Cst816S::Gestures GestureGet();
      private:

        Pinetime::Drivers::Cst816S::TouchInfos info;
        Pinetime::Drivers::Cst816S& touchPanel;
        Pinetime::Components::LittleVgl& lvgl;
        Pinetime::Drivers::Cst816S::Gestures gesture;
        bool isCancelled = false;
        bool gestureReleased = true;
    };
  }
}
