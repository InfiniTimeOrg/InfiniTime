#include "touchhandler/TouchHandler.h"

using namespace Pinetime::Controllers;
using namespace Pinetime::Applications;

namespace {
  TouchEvents ConvertGesture(Pinetime::Drivers::Cst816S::Gestures gesture) {
    switch (gesture) {
      case Pinetime::Drivers::Cst816S::Gestures::SingleTap:
        return TouchEvents::Tap;
      case Pinetime::Drivers::Cst816S::Gestures::LongPress:
        return TouchEvents::LongTap;
      case Pinetime::Drivers::Cst816S::Gestures::DoubleTap:
        return TouchEvents::DoubleTap;
      case Pinetime::Drivers::Cst816S::Gestures::SlideRight:
        return TouchEvents::SwipeRight;
      case Pinetime::Drivers::Cst816S::Gestures::SlideLeft:
        return TouchEvents::SwipeLeft;
      case Pinetime::Drivers::Cst816S::Gestures::SlideDown:
        return TouchEvents::SwipeDown;
      case Pinetime::Drivers::Cst816S::Gestures::SlideUp:
        return TouchEvents::SwipeUp;
      case Pinetime::Drivers::Cst816S::Gestures::None:
      default:
        return TouchEvents::None;
    }
  }
}

Pinetime::Applications::TouchEvents TouchHandler::GestureGet() {
  auto returnGesture = gesture;
  gesture = Pinetime::Applications::TouchEvents::None;
  return returnGesture;
}

bool TouchHandler::ProcessTouchInfo(Drivers::Cst816S::TouchInfos info) {
  if (!info.isValid) {
    return false;
  }
  
  // REPORT configurations (P8b variants) of the fused (usually) Cst716
  // generate multiple "none" gesture events with info.touching == true during the physical gesture.
  // The last event is a e.g. "slide" event with info.touching == true.
  // gestureReleased state does not have to be computed manually, instead it occurs when event != "none".

  // GESTURE configurations (P8a variants) of the fused (usually) Cst716 generate no events during the physical gesture.
  // The only event is a e.g. "slide" event with info.touching == true.
  // gestureReleased state does not have to be computed manually, instead it occurs everytime.

  // DYNAMIC configurations (PineTime) are configured in reporting mode during initialisation.
  // Usually based on the Cst816s, they generate multiple e.g. "slide" gesture events with info.touching == true during the physical
  // gesture. The last of these e.g. "slide" events has info.touching == false. gestureReleased state is computed manually by checking for
  // the transition to info.touching == false.

  // Unfortunately, there is no way to reliably obtain which configuration is used at runtime.
  // In all cases, the event is bubbled up once the gesture is released.

#if defined(DRIVER_TOUCH_REPORT)
  if (info.gesture != Pinetime::Drivers::Cst816S::Gestures::None) {
    gesture = ConvertGesture(info.gesture);
    info.touching = false;
  }
#elif defined(DRIVER_TOUCH_GESTURE)
  if (info.gesture != Pinetime::Drivers::Cst816S::Gestures::None) {
    gesture = ConvertGesture(info.gesture);
  }
#elif defined(DRIVER_TOUCH_DYNAMIC)
  if (info.gesture != Pinetime::Drivers::Cst816S::Gestures::None) {
    if (gestureReleased) {
      if (info.gesture == Pinetime::Drivers::Cst816S::Gestures::SlideDown ||
          info.gesture == Pinetime::Drivers::Cst816S::Gestures::SlideLeft ||
          info.gesture == Pinetime::Drivers::Cst816S::Gestures::SlideUp ||
          info.gesture == Pinetime::Drivers::Cst816S::Gestures::SlideRight ||
          info.gesture == Pinetime::Drivers::Cst816S::Gestures::LongPress) {
        if (info.touching) {
          gesture = ConvertGesture(info.gesture);
          gestureReleased = false;
        }
      } else {
        gesture = ConvertGesture(info.gesture);
      }
    }
  }

  if (!info.touching) {
    gestureReleased = true;
  }
#endif

  currentTouchPoint = {info.x, info.y, info.touching};

  return true;
}

void TouchHandler::UpdateLvglTouchPoint() {
  if (info.touching) {
#if defined(DRIVER_TOUCH_GESTURE)
    // GESTURE config only generates a single event / state change
    // so the LVGL wrapper is used to generate a successive release state update
    lvgl.SetNewTap(info.x, info.y);
#else
    if (!isCancelled) {
      lvgl.SetNewTouchPoint(info.x, info.y, true);
    }
#endif
  } else {
    if (isCancelled) {
      lvgl.SetNewTouchPoint(-1, -1, false);
      isCancelled = false;
    } else {
      lvgl.SetNewTouchPoint(info.x, info.y, false);
    }
  }
}
