#include "touchhandler/TouchHandler.h"

using namespace Pinetime::Controllers;
using namespace Pinetime::Applications;

Pinetime::Applications::TouchEvents TouchHandler::GestureGet() {
  auto returnGesture = gesture;
  gesture = Pinetime::Applications::TouchEvents::None;
  return returnGesture;
}

bool TouchHandler::ProcessTouchInfo(Drivers::Cst816S::TouchInfos info) {
  bool swiping = false;

  if (!info.isValid) {
    return false;
  }

  // Keep the swipe state enabled till we stop touching the screen
  if (currentTouchPoint.swiping) {
    xSwipeDistance += currentTouchPoint.x - info.x;
    ySwipeDistance += currentTouchPoint.y - info.y;
    // Prevent the longTapCounter from increasing while swiping
    longTapCounter = 0;
    swiping = true;
  // Enable the swipe state if we were touching last check, and the previous values are different from the current ones
  } else if (currentTouchPoint.touching && (currentTouchPoint.x != info.x || currentTouchPoint.y != info.y)) {
    swiping = true;
  }


  if (gestureReleased) {
    // Vertical Swipes
    if (std::abs(ySwipeDistance) >= minimumSwipeDistance) {
      if (std::signbit(ySwipeDistance)) {
        gesture = TouchEvents::SwipeDown;
        gestureReleased = false;
      } else {
        gesture = TouchEvents::SwipeUp;
        gestureReleased = false;
      }
    // Horizontal Swipes
    } else if (std::abs(xSwipeDistance) >= minimumSwipeDistance) {
      if (std::signbit(xSwipeDistance)) {
        gesture = TouchEvents::SwipeRight;
        gestureReleased = false;
      } else {
        gesture = TouchEvents::SwipeLeft;
        gestureReleased = false;
      }
    // Long Tap
    } else if (longTapCounter >= longTapHoldTime) {
      gesture = TouchEvents::LongTap;
      gestureReleased = false;
    // Double Tap
    } else if (info.gesture == Pinetime::Drivers::Cst816S::Gestures::DoubleTap) {
      gesture = TouchEvents::DoubleTap;
    // Single Tap
    } else if (info.touching) {
      gesture = TouchEvents::Tap;
      longTapCounter += 1;
    }
  }

  if (!info.touching) {
    gestureReleased = true;
    swiping = false;
    xSwipeDistance = 0;
    ySwipeDistance = 0;
    longTapCounter = 0;
  }

  currentTouchPoint = {info.x, info.y, currentTouchPoint.x, currentTouchPoint.y, info.touching, swiping};

  return true;
}
