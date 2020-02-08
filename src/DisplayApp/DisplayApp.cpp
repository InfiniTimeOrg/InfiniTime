#include "DisplayApp.h"
#include <FreeRTOS.h>
#include <task.h>
#include <libraries/log/nrf_log.h>
#include <boards.h>
#include <nrf_font.h>
#include <hal/nrf_rtc.h>
#include "Components/Gfx/Gfx.h"
#include <queue.h>
#include <Components/DateTime/DateTimeController.h>
#include <drivers/Cst816s.h>
#include <chrono>
#include <string>
#include <lvgl/lvgl.h>
#include "lv_port_disp.h"

using namespace Pinetime::Applications;

DisplayApp::DisplayApp(Pinetime::Drivers::St7789& lcd,
                       Pinetime::Components::Gfx& gfx,
                       Pinetime::Drivers::Cst816S& touchPanel,
                       Controllers::Battery &batteryController,
                       Controllers::Ble &bleController,
                       Controllers::DateTime &dateTimeController) :
        lcd{lcd},
        gfx{gfx},
        touchPanel{touchPanel},
        batteryController{batteryController},
        bleController{bleController},
        dateTimeController{dateTimeController},
        clockScreen{gfx},
        messageScreen{gfx}{
  msgQueue = xQueueCreate(queueSize, itemSize);
  currentScreen = &messageScreen;
}

void DisplayApp::Start() {
  if (pdPASS != xTaskCreate(DisplayApp::Process, "DisplayApp", 1024, this, 0, &taskHandle))
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
}

void DisplayApp::Process(void *instance) {
  auto *app = static_cast<DisplayApp *>(instance);
  NRF_LOG_INFO("DisplayApp task started!");
  app->InitHw();
  lv_init();
  lv_port_disp_init();

  while (1) {
    lv_task_handler();
    app->Refresh();
  }
}

void DisplayApp::InitHw() {
  nrf_gpio_cfg_output(pinLcdBacklight1);
  nrf_gpio_cfg_output(pinLcdBacklight2);
  nrf_gpio_cfg_output(pinLcdBacklight3);
  nrf_gpio_pin_clear(pinLcdBacklight1);
  nrf_gpio_pin_clear(pinLcdBacklight2);
  nrf_gpio_pin_clear(pinLcdBacklight3);


}

uint32_t acc = 0;
uint32_t count = 0;
bool toggle = true;
void DisplayApp::Refresh() {
  TickType_t queueTimeout;
  switch (state) {
    case States::Idle:
      IdleState();
      queueTimeout = portMAX_DELAY;
      break;
    case States::Running:
      RunningState();
      queueTimeout = 1000;
      break;
  }

  Messages msg;
  if (xQueueReceive(msgQueue, &msg, queueTimeout)) {
    switch (msg) {
      case Messages::GoToSleep:
        nrf_gpio_pin_set(pinLcdBacklight3);
        vTaskDelay(100);
        nrf_gpio_pin_set(pinLcdBacklight2);
        vTaskDelay(100);
        nrf_gpio_pin_set(pinLcdBacklight1);
        lcd.DisplayOff();
        lcd.Sleep();
        touchPanel.Sleep();
        state = States::Idle;
        break;
      case Messages::GoToRunning:
        lcd.Wakeup();
        touchPanel.Wakeup();

        lcd.DisplayOn();
        nrf_gpio_pin_clear(pinLcdBacklight3);
        nrf_gpio_pin_clear(pinLcdBacklight2);
        nrf_gpio_pin_clear(pinLcdBacklight1);
        state = States::Running;
        break;
      case Messages::UpdateDateTime:
        break;
      case Messages::UpdateBleConnection:
        clockScreen.SetBleConnectionState(bleController.IsConnected() ? Screens::Clock::BleConnectionStates::Connected : Screens::Clock::BleConnectionStates::NotConnected);
        break;
      case Messages::UpdateBatteryLevel:
        clockScreen.SetBatteryPercentRemaining(batteryController.PercentRemaining());
        break;
      case Messages::TouchEvent:
        if(state != States::Running) break;
        OnTouchEvent();
        break;
    }
  }
}

bool first = true;

void DisplayApp::RunningState() {
  clockScreen.SetCurrentDateTime(dateTimeController.CurrentDateTime());

  if(currentScreen != nullptr) {
    currentScreen->Refresh(first);
    first = false;
  }
}

void DisplayApp::IdleState() {

}

void DisplayApp::PushMessage(DisplayApp::Messages msg) {
  BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(msgQueue, &msg, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken) {
    /* Actual macro used here is port specific. */
    // TODO : should I do something here?
  }
}

static uint16_t pointColor = 0x07e0;
void DisplayApp::OnTouchEvent() {
  auto info = touchPanel.GetTouchInfo();

  if(info.isTouch) {
    gfx.FillRectangle(info.x-10, info.y-10, 20,20, pointColor);
    pointColor+=10;
  }
}
