#include "TouchHandler.h"

using namespace Pinetime::Controllers;

TouchHandler::TouchHandler(Drivers::Cst816S& touchPanel, Components::LittleVgl& lvgl)
  : touchPanel {touchPanel},
    lvgl {lvgl} {
}

void TouchHandler::CancelTap() {
  isCancelled = true;
  lvgl.SetNewTapEvent(-1, -1, false);
}

Pinetime::Drivers::Cst816S::Gestures TouchHandler::GestureGet() {
  auto returnGesture = gesture;
  gesture = Drivers::Cst816S::Gestures::None;
  return returnGesture;
}

void TouchHandler::Start() {
  if (pdPASS != xTaskCreate(TouchHandler::Process, "Touch", 80, this, 0, &taskHandle)) {
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
  }
}

void TouchHandler::Process(void* instance) {
  auto* app = static_cast<TouchHandler*>(instance);
  app->Work();
}

void TouchHandler::Work() {
  Pinetime::Drivers::Cst816S::TouchInfos info;
  while (true) {
    vTaskSuspend(taskHandle);
    info = touchPanel.GetTouchInfo();
    if (systemTask->IsSleeping()) {
      systemTask->PushMessage(System::Messages::TouchWakeUp);
    } else {
      x = info.x;
      y = info.y;
      if (info.finger == 0) {
        lvgl.SetNewTapEvent(info.x, info.y, false);
        prevGesture = Pinetime::Drivers::Cst816S::Gestures::None;
        isCancelled = false;
      } else if (info.finger == 1) {
        if (!isCancelled) {
          lvgl.SetNewTapEvent(info.x, info.y, true);
        }
        if (info.gesture != Pinetime::Drivers::Cst816S::Gestures::None) {
          if (prevGesture != info.gesture) {
            prevGesture = info.gesture;
            gesture = info.gesture;
          }
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
