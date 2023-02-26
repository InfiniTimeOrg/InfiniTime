#pragma once
#include "drivers/Cst816s.h"
#include "displayapp/TouchEvents.h"

namespace Pinetime {
  namespace Controllers {
    class TouchHandler {
    public:
      struct TouchPoint {
        int x;
        int y;
        bool touching;
      };

      bool ProcessTouchInfo(Drivers::Cst816S::TouchInfos info);

      bool IsTouching() const {
        return currentTouchPoint.touching;
      }

      uint8_t GetX() const {
        return currentTouchPoint.x;
      }

      uint8_t GetY() const {
        return currentTouchPoint.y;
      }

      Pinetime::Applications::TouchEvents GestureGet();

    private:
      Pinetime::Applications::TouchEvents gesture;
      TouchPoint currentTouchPoint = {};
      bool gestureReleased = true;
    };
  }
}
