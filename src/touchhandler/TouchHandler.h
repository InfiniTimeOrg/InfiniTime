#pragma once
#include "drivers/Cst816s.h"
#include "displayapp/TouchEvents.h"

namespace Pinetime {
  namespace Drivers {
    class Cst816S;
  }

  namespace Controllers {
    class TouchHandler {
    public:
      explicit TouchHandler(Drivers::Cst816S&);

      bool GetNewTouchInfo();

      bool IsTouching() const {
        return info.touching;
      }

      uint8_t GetX() const {
        return info.x;
      }

      uint8_t GetY() const {
        return info.y;
      }

      Pinetime::Applications::TouchEvents GestureGet();

    private:
      Pinetime::Drivers::Cst816S::TouchInfos info;
      Pinetime::Drivers::Cst816S& touchPanel;
      Pinetime::Applications::TouchEvents gesture;
      bool isCancelled = false;
      bool gestureReleased = true;
    };
  }
}
