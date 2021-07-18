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

void TouchHandler::Start() {
  if (pdPASS != xTaskCreate(TouchHandler::Process, "Touch", 100, this, 0, &taskHandle)) {
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
  }
}

void TouchHandler::Process(void* instance) {
  auto* app = static_cast<TouchHandler*>(instance);
  app->Work();
}

void TouchHandler::Work() {
  bool slideReleased = true;
  while (true) {
    vTaskSuspend(taskHandle);

    info = touchPanel.GetTouchInfo();

    if (info.isValid) {
      if (info.gesture != Pinetime::Drivers::Cst816S::Gestures::None) {
        if (slideReleased) {
          if (info.gesture == Pinetime::Drivers::Cst816S::Gestures::SlideDown ||
              info.gesture == Pinetime::Drivers::Cst816S::Gestures::SlideLeft ||
              info.gesture == Pinetime::Drivers::Cst816S::Gestures::SlideUp ||
              info.gesture == Pinetime::Drivers::Cst816S::Gestures::SlideRight) {
            slideReleased = false;
          }
          gesture = info.gesture;
        }
      }

      if (!systemTask->IsSleeping()) {
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
          slideReleased = true;
        }
      }
      systemTask->OnTouchEvent();
    }
  }
}

void TouchHandler::Register(Pinetime::System::SystemTask* systemTask) {
  this->systemTask = systemTask;
}

void TouchHandler::WakeUp() {
  vTaskResume(taskHandle);
}
