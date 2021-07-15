#include "TouchHandler.h"

using namespace Pinetime::Controllers;

TouchHandler::TouchHandler(Drivers::Cst816S& touchPanel, Components::LittleVgl& lvgl) : touchPanel {touchPanel}, lvgl {lvgl} {
}

void TouchHandler::CancelTap() {
  isCancelled = true;
  lvgl.SetNewTouchPoint(-1, -1, true);
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
  Pinetime::Drivers::Cst816S::TouchInfos info;
  while (true) {
    vTaskSuspend(taskHandle);
    info = touchPanel.GetTouchInfo();
    if (systemTask->IsSleeping()) {
      gesture = info.gesture;
      systemTask->PushMessage(System::Messages::TouchWakeUp);
    } else {
      x = info.x;
      y = info.y;
      if (info.touching) {
        if (!isCancelled) {
          lvgl.SetNewTouchPoint(info.x, info.y, true);
        }
        if (info.gesture != Pinetime::Drivers::Cst816S::Gestures::None) {
          if (prevGesture != info.gesture) {
            prevGesture = info.gesture;
            gesture = info.gesture;
          }
        }
      } else {
        if (isCancelled) {
          lvgl.SetNewTouchPoint(-1, -1, false);
          isCancelled = false;
        } else {
          lvgl.SetNewTouchPoint(info.x, info.y, false);
        }
        prevGesture = Pinetime::Drivers::Cst816S::Gestures::None;
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
