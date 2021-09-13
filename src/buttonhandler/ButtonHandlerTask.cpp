#include "ButtonHandlerTask.h"
#include <libraries/gpiote/app_gpiote.h>

using namespace Pinetime::Controllers;

void ButtonHandler::Start() {
  if (pdPASS != xTaskCreate(ButtonHandler::Process, "Button", 80, this, 0, &taskHandle)) {
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
  }
}

void ButtonHandler::Process(void* instance) {
  auto* app = static_cast<ButtonHandler*>(instance);
  app->Work();
}

void ButtonHandler::Work() {
  TickType_t pressTime = 0;
  TickType_t releaseTime = 0;
  bool handled = false;
  bool lastState = false;
  bool pressed = false;
  bool longerPressed = false;
  static constexpr TickType_t doubleClickTime = pdMS_TO_TICKS(200);
  static constexpr TickType_t longPressTime = pdMS_TO_TICKS(400);
  static constexpr TickType_t longerPressTime = pdMS_TO_TICKS(2000);

  while (true) {
    vTaskSuspend(taskHandle);

    pressed = nrf_gpio_pin_read(PinMap::Button);
    if (pressed) {
      systemTask->PushMessage(System::Messages::ReloadIdleTimer);
      while (true) {
        if (pressed) {
          if (pressed != lastState) {
            if (xTaskGetTickCount() - pressTime < longPressTime) {
              if (xTaskGetTickCount() - releaseTime < doubleClickTime) {
                if (!handled) {
                  handled = true;
                  systemTask->PushMessage(System::Messages::OnButtonDoubleClicked);
                }
              }
            }
            pressTime = xTaskGetTickCount();
            lastState = true;

            // This is for faster wakeup, sacrificing longpress and
            // doubleclick handling while sleeping
            if (systemTask->IsSleeping()) {
              systemTask->PushMessage(System::Messages::GoToRunning);
              handled = true;
            }
          } else if (xTaskGetTickCount() - pressTime >= longPressTime) {
            if (xTaskGetTickCount() - pressTime >= longerPressTime) {
              if (!longerPressed) {
                longerPressed = true;
                systemTask->PushMessage(System::Messages::OnButtonLongerPressed);
              }
            }
            if (!handled) {
              handled = true;
              systemTask->PushMessage(System::Messages::OnButtonLongPressed);
            }
          }
        } else {
          if (pressed != lastState) {
            systemTask->PushMessage(System::Messages::ReloadIdleTimer);
            releaseTime = xTaskGetTickCount();
            lastState = false;
            longerPressed = false;
            if (handled) {
              handled = false;
              break;
            }
          } else if (xTaskGetTickCount() - releaseTime >= doubleClickTime) {
            if (!handled) {
              systemTask->PushMessage(System::Messages::OnButtonPushed);
              break;
            }
          }
        }
        vTaskDelay(20);
        pressed = nrf_gpio_pin_read(PinMap::Button);
      }
    }
  }
}

void ButtonHandler::Register(Pinetime::System::SystemTask* systemTask) {
  this->systemTask = systemTask;
}

void ButtonHandler::WakeUp() {
  vTaskResume(taskHandle);
}
