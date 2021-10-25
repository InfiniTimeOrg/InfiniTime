#include "ButtonHandler.h"

using namespace Pinetime::Controllers;

void ButtonTimerCallback(TimerHandle_t xTimer) {
  auto* buttonHandler = static_cast<ButtonHandler*>(pvTimerGetTimerID(xTimer));
  buttonHandler->HandleEvent(ButtonHandler::Events::Timer);
}

void ButtonHandler::Init(Pinetime::System::SystemTask* systemTask) {
  this->systemTask = systemTask;
  buttonTimer = xTimerCreate("buttonTimer", 0, pdFALSE, this, ButtonTimerCallback);
}

void ButtonHandler::HandleEvent(Events event) {
  static constexpr TickType_t doubleClickTime = pdMS_TO_TICKS(200);
  static constexpr TickType_t longPressTime = pdMS_TO_TICKS(400);
  static constexpr TickType_t longerPressTime = pdMS_TO_TICKS(2000);

  if (systemTask->IsSleeping()) {
    // This is for faster wakeup, sacrificing special longpress and doubleclick handling while sleeping
    systemTask->PushMessage(System::Messages::GoToRunning);
  } else {
    systemTask->PushMessage(System::Messages::ReloadIdleTimer);
  }

  if (event == Events::Press) {
    buttonPressed = true;
  } else if (event == Events::Release) {
    releaseTime = xTaskGetTickCount();
    buttonPressed = false;
  }

  switch (state) {
    case States::Idle:
      if (event == Events::Press) {
        xTimerChangePeriod(buttonTimer, doubleClickTime, 0);
        xTimerStart(buttonTimer, 0);
        state = States::Pressed;
      }
      break;
    case States::Pressed:
      if (event == Events::Press) {
        if (xTaskGetTickCount() - releaseTime < doubleClickTime) {
          systemTask->PushMessage(System::Messages::OnButtonDoubleClicked);
          xTimerStop(buttonTimer, 0);
          state = States::Idle;
        }
      } else if (event == Events::Release) {
        xTimerChangePeriod(buttonTimer, doubleClickTime, 0);
        xTimerStart(buttonTimer, 0);
      } else if (event == Events::Timer) {
        if (buttonPressed) {
          xTimerChangePeriod(buttonTimer, longPressTime - doubleClickTime, 0);
          xTimerStart(buttonTimer, 0);
          state = States::Holding;
        } else {
          systemTask->PushMessage(System::Messages::OnButtonPushed);
          state = States::Idle;
        }
      }
      break;
    case States::Holding:
      if (event == Events::Release) {
        systemTask->PushMessage(System::Messages::OnButtonPushed);
        xTimerStop(buttonTimer, 0);
        state = States::Idle;
      } else if (event == Events::Timer) {
        xTimerChangePeriod(buttonTimer, longerPressTime - longPressTime - doubleClickTime, 0);
        xTimerStart(buttonTimer, 0);
        systemTask->PushMessage(System::Messages::OnButtonLongPressed);
        state = States::LongHeld;
      }
      break;
    case States::LongHeld:
      if (event == Events::Release) {
        xTimerStop(buttonTimer, 0);
        state = States::Idle;
      } else if (event == Events::Timer) {
        systemTask->PushMessage(System::Messages::OnButtonLongerPressed);
        state = States::Idle;
      }
      break;
  }
}
