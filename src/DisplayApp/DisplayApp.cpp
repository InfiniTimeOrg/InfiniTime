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

using namespace Pinetime::Applications;

char const *DisplayApp::DaysString[] = {
        "",
        "MONDAY",
        "TUESDAY",
        "WEDNESDAY",
        "THURSDAY",
        "FRIDAY",
        "SATURDAY",
        "SUNDAY"
};

char const *DisplayApp::MonthsString[] = {
        "",
        "JAN",
        "FEB",
        "MAR",
        "APR",
        "MAY",
        "JUN",
        "JUL",
        "AUG",
        "SEP",
        "OCT",
        "NOV",
        "DEC"
};

DisplayApp::DisplayApp(Controllers::Battery &batteryController,
                       Controllers::Ble &bleController,
                       Controllers::DateTime &dateTimeController) :
        batteryController{batteryController},
        bleController{bleController},
        dateTimeController{dateTimeController} {
  msgQueue = xQueueCreate(queueSize, itemSize);
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
  gfx->DrawChar(&largeFont, '0', &x, 78, 0xffff);

  x = 61;
  gfx->DrawChar(&largeFont, '0', &x, 78, 0xffff);

  x = 94;
  gfx->DrawChar(&largeFont, ':', &x, 78, 0xffff);

  x = 127;
  gfx->DrawChar(&largeFont, '0', &x, 78, 0xffff);

  x = 181;
  gfx->DrawChar(&largeFont, '0', &x, 78, 0xffff);

  gfx->DrawString(10, 0, 0x0000, "BLE", &smallFont, false);
  gfx->DrawString(20, 180, 0xffff, "", &smallFont, false);

  touchPanel.Init();
}

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
        nrf_gpio_pin_set(23);
        vTaskDelay(100);
        nrf_gpio_pin_set(22);
        vTaskDelay(100);
        nrf_gpio_pin_set(14);
        lcd->DisplayOff();
        state = States::Idle;
        break;
      case Messages::GoToRunning:
        lcd->DisplayOn();
        nrf_gpio_pin_clear(23);
        nrf_gpio_pin_clear(22);
        nrf_gpio_pin_clear(14);
        state = States::Running;
        break;
      case Messages::UpdateDateTime:
        deltaSeconds = nrf_rtc_counter_get(portNRF_RTC_REG) / 1000;
        this->seconds = dateTimeController.Seconds();
        this->minutes = dateTimeController.Minutes();
        this->hours = dateTimeController.Hours();
        dateUpdated = true;
        break;
      case Messages::UpdateBleConnection:
        bleConnectionUpdated = true;
        break;
      case Messages::UpdateBatteryLevel:
        batteryLevelUpdated = true;
        break;
      case Messages::TouchEvent:
        if(state != States::Running) break;
        OnTouchEvent();
        break;
    }
  }
}

void DisplayApp::RunningState() {
  uint32_t systick_counter = nrf_rtc_counter_get(portNRF_RTC_REG);

  if (batteryLevelUpdated) {
    char batteryChar[11];
    uint16_t newBatteryValue = batteryController.PercentRemaining();
    newBatteryValue = (newBatteryValue > 100) ? 100 : newBatteryValue;
    newBatteryValue = (newBatteryValue < 0) ? 0 : newBatteryValue;

    batteryLevelUpdated = false;
    sprintf(batteryChar, "BAT: %d%%", newBatteryValue);
    gfx->DrawString((240 - 108), 0, 0xffff, batteryChar, &smallFont, false);
  }

  if (bleConnectionUpdated) {
    bleConnectionUpdated = false;
    uint16_t color = (bleController.IsConnected()) ? 0xffff : 0x0000;
    gfx->DrawString(10, 0, color, "BLE", &smallFont, false);
  }

  auto raw = systick_counter / 1000;
  auto currentDeltaSeconds = raw - deltaSeconds;

  auto deltaMinutes = (currentDeltaSeconds / 60);
  auto currentMinutes = minutes + deltaMinutes;

  auto deltaHours = currentMinutes / 60;
  currentMinutes -= (deltaHours * 60);
  auto currentHours = hours + deltaHours;

  char minutesChar[3];
  sprintf(minutesChar, "%02d", currentMinutes);

  char hoursChar[3];
  sprintf(hoursChar, "%02d", currentHours);

  uint8_t x = 7;
  if (hoursChar[0] != currentChar[0]) {
    gfx->DrawChar(&largeFont, hoursChar[0], &x, 78, 0xffff);
    currentChar[0] = hoursChar[0];
  }

  x = 61;
  if (hoursChar[1] != currentChar[1]) {
    gfx->DrawChar(&largeFont, hoursChar[1], &x, 78, 0xffff);
    currentChar[1] = hoursChar[1];
  }

  x = 127;
  if (minutesChar[0] != currentChar[2]) {
    gfx->DrawChar(&largeFont, minutesChar[0], &x, 78, 0xffff);
    currentChar[2] = minutesChar[0];
  }

  x = 181;
  if (minutesChar[1] != currentChar[3]) {
    gfx->DrawChar(&largeFont, minutesChar[1], &x, 78, 0xffff);
    currentChar[3] = minutesChar[1];
  }

  if (dateUpdated) {
    auto year = dateTimeController.Year();
    auto month = dateTimeController.Month();
    auto day = dateTimeController.Day();
    auto dayOfWeek = dateTimeController.DayOfWeek();

    char dateStr[22];
    sprintf(dateStr, "%s %d %s %d", DayOfWeekToString(dayOfWeek), day, MonthToString(month), year);
    gfx->DrawString(10, 180, 0xffff, dateStr, &smallFont, false);
    dateUpdated = false;
  }
}

const char *DisplayApp::MonthToString(Pinetime::Controllers::DateTime::Months month) {
  return DisplayApp::MonthsString[static_cast<uint8_t>(month)];
}

const char *DisplayApp::DayOfWeekToString(Pinetime::Controllers::DateTime::Days dayOfWeek) {
  return DisplayApp::DaysString[static_cast<uint8_t>(dayOfWeek)];
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
    lcd->FillRectangle(info.x-10, info.y-10, 20,20, pointColor);
    pointColor+=10;
  }
}
