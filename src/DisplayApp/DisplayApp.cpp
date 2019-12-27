#include "DisplayApp.h"
#include <FreeRTOS.h>
#include <task.h>
#include <libraries/log/nrf_log.h>
#include <boards.h>
#include <nrf_font.h>
#include <hal/nrf_rtc.h>
#include "Components/Gfx/Gfx.h"
#include <queue.h>

using namespace Pinetime::Applications;

DisplayApp::DisplayApp(Pinetime::Controllers::Battery &batteryController, Pinetime::Controllers::Ble &bleController) :
        batteryController{batteryController},
        bleController{bleController} {
  msgQueue = xQueueCreate(queueSize, itemSize);
}

void DisplayApp::Start() {
  if (pdPASS != xTaskCreate(DisplayApp::Process, "DisplayApp", 256, this, 0, &taskHandle))
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
}

void DisplayApp::Process(void *instance) {
  auto* app = static_cast<DisplayApp*>(instance);
  NRF_LOG_INFO("DisplayApp task started!");
  app->InitHw();

  while (1) {
    app->Refresh();
  }
}

void DisplayApp::InitHw() {
  nrf_gpio_cfg_output(14);
  nrf_gpio_cfg_output(22);
  nrf_gpio_cfg_output(23);
  nrf_gpio_pin_clear(14);
  nrf_gpio_pin_clear(22);
  nrf_gpio_pin_clear(23);

  Drivers::SpiMaster::Parameters params;
  params.bitOrder = Drivers::SpiMaster::BitOrder::Msb_Lsb;
  params.mode = Drivers::SpiMaster::Modes::Mode3;
  params.Frequency = Drivers::SpiMaster::Frequencies::Freq8Mhz;
  params.pinCSN = 25;
  params.pinMISO = 4;
  params.pinMOSI = 3;
  params.pinSCK = 2;
  spi.Init(Drivers::SpiMaster::SpiModule::SPI0, params);

  lcd.reset(new Drivers::St7789(spi, 18));
  gfx.reset(new Components::Gfx(*lcd.get()));
  gfx->ClearScreen();

  uint8_t x = 7;
  gfx->DrawChar(&largeFont , '0', &x, 78, 0xffff);

  x = 61;
  gfx->DrawChar(&largeFont, '0', &x, 78, 0xffff);

  x = 94;
  gfx->DrawChar(&largeFont, ':', &x, 78, 0xffff);

  x = 127;
  gfx->DrawChar(&largeFont, '0', &x, 78, 0xffff);

  x = 181;
  gfx->DrawChar(&largeFont, '0', &x, 78, 0xffff);

  gfx->DrawString(10, 0, 0x0000, "BLE", &smallFont, false);
  gfx->DrawString(20, 180, 0xffff, "FRIDAY 27 DEC 2019", &smallFont, false);
}

void DisplayApp::Refresh() {
  TickType_t queueTimeout;
  switch(state) {
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
  if(xQueueReceive( msgQueue, &msg, queueTimeout)) {
    switch(msg) {
      case Messages::GoToSleep:
        nrf_gpio_pin_set(23);
        vTaskDelay(100);
        nrf_gpio_pin_set(22);
        vTaskDelay(100);
        nrf_gpio_pin_set(14);
        state = States::Idle;
        break;
      case Messages::GoToRunning:
        nrf_gpio_pin_clear(23);
        nrf_gpio_pin_clear(22);
        nrf_gpio_pin_clear(14);
        state = States::Running;
        break;
    }
  }
}

void DisplayApp::Minutes(uint8_t m) {
  // TODO yeah, I know, race condition...
  minutes = m;
}

void DisplayApp::Hours(uint8_t h) {
  // TODO yeah, I know, race condition too...
  hours = h;
}

void DisplayApp::SetTime(uint8_t minutes, uint8_t hours) {
  deltaSeconds = nrf_rtc_counter_get(portNRF_RTC_REG) / 1000;
  this->minutes = minutes;
  this->hours = hours;
}

void DisplayApp::RunningState() {
  uint32_t systick_counter = nrf_rtc_counter_get(portNRF_RTC_REG);

  char batteryChar[11];
  uint16_t newBatteryValue = batteryController.PercentRemaining();
  newBatteryValue = (newBatteryValue>100) ? 100 : newBatteryValue;
  newBatteryValue = (newBatteryValue<0) ? 0 : newBatteryValue;
  if(newBatteryValue != battery) {
    battery = newBatteryValue;
    sprintf(batteryChar, "BAT: %d%%", battery);
    gfx->DrawString((240-108), 0, 0xffff, batteryChar, &smallFont, false);
  }

  bool newIsBleConnected = bleController.IsConnected();
  if(newIsBleConnected != bleConnected) {
    bleConnected = newIsBleConnected;
    uint16_t color = (bleConnected) ? 0xffff : 0x0000;
    gfx->DrawString(10, 0, color, "BLE", &smallFont, false);
  }

  auto raw = systick_counter / 1000;
  auto currentDeltaSeconds = raw - deltaSeconds;


  auto deltaMinutes = (currentDeltaSeconds / 60);
  auto currentMinutes = minutes + deltaMinutes;

  auto deltaHours = currentMinutes / 60;
  currentMinutes -= (deltaHours * 60);

//
//   TODO make this better!
//  minutes = raw / 60;
//  seconds = raw - (minutes*60);


  auto currentHours = hours + deltaHours;



  char minutesChar[3];
  sprintf(minutesChar, "%02d", currentMinutes);

  char hoursChar[3];
  sprintf(hoursChar, "%02d", currentHours);

  uint8_t x = 7;
  if(hoursChar[0] != currentChar[0]) {
    gfx->DrawChar(&largeFont, hoursChar[0], &x, 78, 0xffff);
    currentChar[0] = hoursChar[0];
  }

  x = 61;
  if(hoursChar[1] != currentChar[1]) {
    gfx->DrawChar(&largeFont, hoursChar[1], &x, 78, 0xffff);
    currentChar[1] = hoursChar[1];
  }

  x = 127;
  if(minutesChar[0] != currentChar[2]) {
    gfx->DrawChar(&largeFont, minutesChar[0], &x, 78, 0xffff);
    currentChar[2] = minutesChar[0];
  }

  x = 181;
  if(minutesChar[1] != currentChar[3]) {
    gfx->DrawChar(&largeFont, minutesChar[1], &x, 78, 0xffff);
    currentChar[3] = minutesChar[1];
  }
}

void DisplayApp::IdleState() {

}

void DisplayApp::PushMessage(DisplayApp::Messages msg) {
  BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR( msgQueue, &msg, &xHigherPriorityTaskWoken );

  /* Now the buffer is empty we can switch context if necessary. */
  if(xHigherPriorityTaskWoken) {
    /* Actual macro used here is port specific. */
    // TODO : should I do something here?
  }
}
