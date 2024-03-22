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

TouchHandler::TouchProcessReply TouchHandler::ProcessTouchInfo(Drivers::Cst816S::TouchInfos info, bool buttonUnlocksOn) {
  if (!info.isValid) {
    return TouchHandler::TouchProcessReply::NoAction;
  }

  // Only a single gesture per touch
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

  currentTouchPoint = {info.x, info.y, info.touching};

  // if the watch was just woken by touch and button must be used to unlock, ignore the first touch event which
  // is the touch event that woke the watch. Otherwise the lock-popup will be displayed
  if (buttonUnlocksOn && ignoreNextTouchEvent) {
    ignoreNextTouchEvent = false;
    return TouchHandler::TouchProcessReply::NoAction;

  } else if (!buttonUnlocksOn || wokenBy != WokenBy::WakeUpAction) {

    // if we get to here TouchEvents is allowed and the "ButtonUnlocks" requirement can be overridden
    wokenBy = WokenBy::Other;

    return TouchHandler::TouchProcessReply::TouchEvent;
  } else {
    return TouchHandler::TouchProcessReply::IgnoreTouchPopup;
  }
}
