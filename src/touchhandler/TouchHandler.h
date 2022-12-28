#pragma once
#include "port/TouchPanel.h"
#include "displayapp/TouchEvents.h"

namespace Pinetime {
  namespace Components {
    class LittleVgl;
  }

  namespace Controllers {
    class TouchHandler {
    public:
      explicit TouchHandler(Pinetime::Drivers::TouchPanel&, Components::LittleVgl&);
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
      Pinetime::Applications::TouchEvents GestureGet();

    private:
      Pinetime::Drivers::TouchPanels::TouchInfos info;
      Pinetime::Drivers::TouchPanel& touchPanel;
      Pinetime::Components::LittleVgl& lvgl;
      Pinetime::Applications::TouchEvents gesture;
      bool isCancelled = false;
      bool gestureReleased = true;
    };
  }
}
