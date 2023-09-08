#pragma once
#include "drivers/Cst816s.h"
#include "displayapp/TouchEvents.h"

namespace Pinetime {
  namespace Controllers {
    class TouchHandler {
    public:
      // Enum describes how the watch was woken:
      //  * WakeUpAction: The actions selected in the wakeup settings, single/double tap, raise, shake
      //  * Button: The hardware button
      //  * Other: Other things that can wake the watch up, eg. apps and notifications.
      enum class WokenBy { WakeUpAction, Button, Other };

      // Enum describes how the reply from ProcessTouchInfo should be interpreted:
      //  * NoAction: Do nothing, ignore input.
      //  * TouchEvent: The input should be treated as a normal touch event.
      //  * IgnoreTouchPopup: Show the popup for when ignoring touvh input.
      enum class TouchProcessReply { NoAction, TouchEvent, IgnoreTouchPopup };

      struct TouchPoint {
        int x;
        int y;
        bool touching;
      };

      TouchProcessReply ProcessTouchInfo(Drivers::Cst816S::TouchInfos info, bool buttonUnlocksOn);

      bool IsTouching() const {
        return currentTouchPoint.touching;
      }

      uint8_t GetX() const {
        return currentTouchPoint.x;
      }

      uint8_t GetY() const {
        return currentTouchPoint.y;
      }

      void SetIgnoreTouchPopupHidden(bool hidden) {
        ignoreTouchPopupHidden = hidden;
      }

      bool IsIgnoreTouchPopupHidden() {
        return ignoreTouchPopupHidden;
      }

      void SetWokenBy(WokenBy woken, bool ifButtonUnlocksIgnoreTouch) {
        wokenBy = woken;
        ignoreNextTouchEvent = ifButtonUnlocksIgnoreTouch;
      }

      Pinetime::Applications::TouchEvents GestureGet();

    private:
      Pinetime::Applications::TouchEvents gesture;
      TouchPoint currentTouchPoint = {};
      bool gestureReleased = true;

      WokenBy wokenBy;
      bool ignoreNextTouchEvent = false;
      bool ignoreTouchPopupHidden = true;
    };
  }
}
