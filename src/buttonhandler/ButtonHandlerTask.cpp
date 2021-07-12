#include "ButtonHandlerTask.h"
#include <libraries/gpiote/app_gpiote.h>

using namespace Pinetime::Controllers;

namespace {
  // copied from Stopwatch
  TickType_t calculateDelta(const TickType_t startTime, const TickType_t currentTime) {
    TickType_t delta = 0;
    // Take care of overflow
    if (startTime > currentTime) {
      delta = 0xffffffff - startTime;
      delta += (currentTime + 1);
    } else {
      delta = currentTime - startTime;
    }
    return delta;
  }
}

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
  static constexpr TickType_t doubleClickTime = pdMS_TO_TICKS(200);
  static constexpr TickType_t longPressTime = pdMS_TO_TICKS(400);
  static constexpr uint8_t pinButton = 13;

  while (true) {
    vTaskSuspend(taskHandle);

    pressed = nrf_gpio_pin_read(pinButton);
    if (pressed) {
      while (true) {
        if (pressed) {
          if (pressed != lastState) {
            pressTime = xTaskGetTickCount();
            lastState = true;

            // This is for faster wakeup, sacrificing longpress and
            // doubleclick handling while sleeping
            if (systemTask->IsSleeping()) {
              systemTask->PushMessage(System::Messages::GoToRunning);
              handled = true;
            }

            if (calculateDelta(releaseTime, xTaskGetTickCount()) < doubleClickTime) {
              if (!handled) {
                handled = true;
                systemTask->OnButtonDoubleClicked();
              }
            }
          } else if (calculateDelta(pressTime, xTaskGetTickCount()) >= longPressTime) {
            if (!handled) {
              handled = true;
              systemTask->OnButtonLongPressed();
            }
          }
        } else {
          if (pressed != lastState) {
            releaseTime = xTaskGetTickCount();
            lastState = false;
            if (handled) {
              handled = false;
              break;
            }
          } else if (calculateDelta(releaseTime, xTaskGetTickCount()) >= doubleClickTime) {
            if (!handled) {
              systemTask->OnButtonPushed();
              break;
            }
          }
        }
        vTaskDelay(20);
        pressed = nrf_gpio_pin_read(pinButton);
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
