#pragma once
#include "drivers/Cst816s.h"
#include "displayapp/TouchEvents.h"

#include <cmath>

namespace Pinetime {
  namespace Controllers {
    class TouchHandler {
    public:
      struct TouchPoint {
        int x;
        int y;
        int previousX;
        int previousY;
        bool touching;
        bool swiping;
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
      const uint8_t minimumSwipeDistance = 65;
      const uint8_t longTapHoldTime = 50;

      Pinetime::Applications::TouchEvents gesture;
      TouchPoint currentTouchPoint = {};
      bool gestureReleased = true;
      // Total distance for a given swipe
      int16_t xSwipeDistance = 0;
      int16_t ySwipeDistance = 0;
      uint16_t longTapCounter = 0;
    };
  }
}
