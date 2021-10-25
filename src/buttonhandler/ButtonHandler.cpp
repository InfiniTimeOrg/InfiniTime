#include "ButtonHandler.h"

using namespace Pinetime::Controllers;

void ButtonTimerCallback(TimerHandle_t xTimer) {
  auto* buttonHandler = static_cast<ButtonHandler*>(pvTimerGetTimerID(xTimer));
  buttonHandler->HandleEvent(ButtonHandler::Timer);
}

void ButtonHandler::Init(Pinetime::System::SystemTask* systemTask) {
  this->systemTask = systemTask;
  buttonTimer = xTimerCreate("buttonTimer", 0, pdFALSE, this, ButtonTimerCallback);
}

void ButtonHandler::HandleEvent(events event) {
  static constexpr TickType_t doubleClickTime = pdMS_TO_TICKS(200);
  static constexpr TickType_t longPressTime = pdMS_TO_TICKS(400);
  static constexpr TickType_t longerPressTime = pdMS_TO_TICKS(2000);

  if (systemTask->IsSleeping()) {
    // This is for faster wakeup, sacrificing special longpress and doubleclick handling while sleeping
    systemTask->PushMessage(System::Messages::GoToRunning);
  } else {
    systemTask->PushMessage(System::Messages::ReloadIdleTimer);
  }

  if (event == Press) {
    buttonPressed = true;
  } else if (event == Release) {
    releaseTime = xTaskGetTickCount();
    buttonPressed = false;
  }

  switch (state) {
    case Idle:
      if (event == Press) {
        xTimerChangePeriod(buttonTimer, doubleClickTime, 0);
        xTimerStart(buttonTimer, 0);
        state = Pressed;
      }
      break;
    case Pressed:
      if (event == Press) {
        if (xTaskGetTickCount() - releaseTime < doubleClickTime) {
          systemTask->PushMessage(System::Messages::OnButtonDoubleClicked);
          xTimerStop(buttonTimer, 0);
          state = Idle;
        }
      } else if (event == Release) {
        xTimerChangePeriod(buttonTimer, doubleClickTime, 0);
        xTimerStart(buttonTimer, 0);
      } else if (event == Timer) {
        if (buttonPressed) {
          xTimerChangePeriod(buttonTimer, longPressTime - doubleClickTime, 0);
          xTimerStart(buttonTimer, 0);
          state = Holding;
        } else {
          systemTask->PushMessage(System::Messages::OnButtonPushed);
          state = Idle;
        }
      }
      break;
    case Holding:
      if (event == Release) {
        systemTask->PushMessage(System::Messages::OnButtonPushed);
        xTimerStop(buttonTimer, 0);
        state = Idle;
      } else if (event == Timer) {
        xTimerChangePeriod(buttonTimer, longerPressTime - longPressTime - doubleClickTime, 0);
        xTimerStart(buttonTimer, 0);
        systemTask->PushMessage(System::Messages::OnButtonLongPressed);
        state = LongHeld;
      }
      break;
    case LongHeld:
      if (event == Release) {
        xTimerStop(buttonTimer, 0);
        state = Idle;
      } else if (event == Timer) {
        systemTask->PushMessage(System::Messages::OnButtonLongerPressed);
        state = Idle;
      }
      break;
  }
}
