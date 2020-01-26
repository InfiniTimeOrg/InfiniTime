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
        messageScreen{gfx} {
  msgQueue = xQueueCreate(queueSize, itemSize);
  currentScreen = &clockScreen;
}

void DisplayApp::Start() {
  if (pdPASS != xTaskCreate(DisplayApp::Process, "DisplayApp", 256, this, 0, &taskHandle))
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
}

void DisplayApp::Process(void *instance) {
  auto *app = static_cast<DisplayApp *>(instance);
  NRF_LOG_INFO("DisplayApp task started!");
  app->InitHw();
  while (1) {
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

  currentScreen->Refresh(true);
}

uint32_t acc = 0;
uint32_t count = 0;
bool toggle = true;
void DisplayApp::Refresh() {
#if 0
  uint32_t before = nrf_rtc_counter_get(portNRF_RTC_REG);
  if(toggle) {
    gfx.FillRectangle(0,0,240,240,0x0000);
  } else {
    gfx.FillRectangle(0,0,240,240,0xffff);
  }
  uint32_t after = nrf_rtc_counter_get(portNRF_RTC_REG);

  acc += (after - before);
  if((count % 10) == 0) {
    NRF_LOG_INFO("DRAW : %d ms", (uint32_t)(acc/10));
    acc = 0;
  }
  count++;
  toggle = !toggle;
#endif

#if 1
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
#endif
}

void DisplayApp::RunningState() {
  clockScreen.SetCurrentDateTime(dateTimeController.CurrentDateTime());

//  if(currentScreen != nullptr) {
//    currentScreen->Refresh(false);
//  }

  if(currentScreen != nullptr) {
    currentScreen->Refresh(true);
  }

  if(screenState) {
    currentScreen = &clockScreen;
  } else {
    currentScreen = &messageScreen;
  }
  screenState = !screenState;
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
