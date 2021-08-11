#include "TouchHandler.h"

using namespace Pinetime::Controllers;

TouchHandler::TouchHandler(Drivers::Cst816S& touchPanel, Components::LittleVgl& lvgl) : touchPanel {touchPanel}, lvgl {lvgl} {
}

void TouchHandler::CancelTap() {
  if (info.touching) {
    isCancelled = true;
    lvgl.SetNewTouchPoint(-1, -1, true);
  }
}

Pinetime::Drivers::Cst816S::Gestures TouchHandler::GestureGet() {
  auto returnGesture = gesture;
  gesture = Drivers::Cst816S::Gestures::None;
  return returnGesture;
}

bool TouchHandler::GetNewTouchInfo() {
  info = touchPanel.GetTouchInfo();

  if (!info.isValid) {
    return false;
  }

  if (info.gesture != Pinetime::Drivers::Cst816S::Gestures::None) {
    if (gestureReleased) {
      if (info.gesture == Pinetime::Drivers::Cst816S::Gestures::SlideDown ||
          info.gesture == Pinetime::Drivers::Cst816S::Gestures::SlideLeft ||
          info.gesture == Pinetime::Drivers::Cst816S::Gestures::SlideUp ||
          info.gesture == Pinetime::Drivers::Cst816S::Gestures::SlideRight ||
          info.gesture == Pinetime::Drivers::Cst816S::Gestures::LongPress) {
        if (info.touching) {
          gesture = info.gesture;
          gestureReleased = false;
        }
      } else {
        gesture = info.gesture;
      }
    }
  }

  if (!info.touching) {
    gestureReleased = true;
  }

  return true;
}

void TouchHandler::UpdateLvglTouchPoint() {
  if (info.touching) {
    if (!isCancelled) {
      lvgl.SetNewTouchPoint(info.x, info.y, true);
    }
  } else {
    if (isCancelled) {
      lvgl.SetNewTouchPoint(-1, -1, false);
      isCancelled = false;
    } else {
      lvgl.SetNewTouchPoint(info.x, info.y, false);
    }
  }
}
