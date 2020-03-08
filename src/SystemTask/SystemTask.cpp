#include <libraries/log/nrf_log.h>
#include <libraries/gpiote/app_gpiote.h>
#include <drivers/Cst816s.h>
#include <DisplayApp/LittleVgl.h>
#include <hal/nrf_rtc.h>
#include <BLE/BleManager.h>
#include <softdevice/common/nrf_sdh_freertos.h>
#include "SystemTask.h"
#include "../main.h"
using namespace Pinetime::System;

SystemTask::SystemTask(Pinetime::Drivers::SpiMaster &spi, Pinetime::Drivers::St7789 &lcd,
                       Pinetime::Drivers::Cst816S &touchPanel, Pinetime::Components::LittleVgl &lvgl,
                       Pinetime::Controllers::Battery &batteryController, Pinetime::Controllers::Ble &bleController,
                       Pinetime::Controllers::DateTime& dateTimeController) :
                       spi{spi}, lcd{lcd}, touchPanel{touchPanel}, lvgl{lvgl}, batteryController{batteryController}, bleController{bleController}, dateTimeController{dateTimeController} {
  systemTaksMsgQueue = xQueueCreate(10, 1);
}

void SystemTask::Start() {
  if (pdPASS != xTaskCreate(SystemTask::Process, "MAIN", 350, this, 0, &taskHandle))
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
}

void SystemTask::Process(void *instance) {
  auto *app = static_cast<SystemTask *>(instance);
  NRF_LOG_INFO("SystemTask task started!");
  app->Work();
}

void SystemTask::Work() {
  watchdog.Setup(7);
  watchdog.Start();
  NRF_LOG_INFO("Last reset reason : %s", Pinetime::Drivers::Watchdog::ResetReasonToString(watchdog.ResetReason()));
  APP_GPIOTE_INIT(2);
  bool erase_bonds=false;
//  ble_manager_init_peer_manager();
//  nrf_sdh_freertos_init(ble_manager_start_advertising, &erase_bonds);

  spi.Init();
  lcd.Init();
  touchPanel.Init();
  batteryController.Init();

  displayApp.reset(new Pinetime::Applications::DisplayApp(lcd, lvgl, touchPanel, batteryController, bleController, dateTimeController, *this));
  displayApp->Start();

  batteryController.Update();
  displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::UpdateBatteryLevel);

  nrf_gpio_cfg_sense_input(pinButton, (nrf_gpio_pin_pull_t)GPIO_PIN_CNF_PULL_Pulldown, (nrf_gpio_pin_sense_t)GPIO_PIN_CNF_SENSE_High);
  nrf_gpio_cfg_output(15);
  nrf_gpio_pin_set(15);

  nrfx_gpiote_in_config_t pinConfig;
  pinConfig.skip_gpio_setup = true;
  pinConfig.hi_accuracy = false;
  pinConfig.is_watcher = false;
  pinConfig.sense = (nrf_gpiote_polarity_t)NRF_GPIOTE_POLARITY_HITOLO;
  pinConfig.pull = (nrf_gpio_pin_pull_t)GPIO_PIN_CNF_PULL_Pulldown;

  nrfx_gpiote_in_init(pinButton, &pinConfig, nrfx_gpiote_evt_handler);

  nrf_gpio_cfg_sense_input(pinTouchIrq, (nrf_gpio_pin_pull_t)GPIO_PIN_CNF_PULL_Pullup, (nrf_gpio_pin_sense_t)GPIO_PIN_CNF_SENSE_Low);

  pinConfig.skip_gpio_setup = true;
  pinConfig.hi_accuracy = false;
  pinConfig.is_watcher = false;
  pinConfig.sense = (nrf_gpiote_polarity_t)NRF_GPIOTE_POLARITY_HITOLO;
  pinConfig.pull = (nrf_gpio_pin_pull_t)GPIO_PIN_CNF_PULL_Pullup;

  nrfx_gpiote_in_init(pinTouchIrq, &pinConfig, nrfx_gpiote_evt_handler);


  while(true) {
    uint8_t msg;
    if (xQueueReceive(systemTaksMsgQueue, &msg, isSleeping?2500 : 1000)) {
      Messages message = static_cast<Messages >(msg);
      switch(message) {
        case Messages::GoToRunning: isSleeping = false; break;
        case Messages::GoToSleep:
          NRF_LOG_INFO("[SystemTask] Going to sleep");
          displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::GoToSleep);
          isSleeping = true; break;
        default: break;
      }
    }
    uint32_t systick_counter = nrf_rtc_counter_get(portNRF_RTC_REG);
    dateTimeController.UpdateTime(systick_counter);

    if(!nrf_gpio_pin_read(pinButton))
      watchdog.Kick();
  }
}

void SystemTask::OnButtonPushed() {

  if(!isSleeping) {
    NRF_LOG_INFO("[SystemTask] Button pushed");
    displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::ButtonPushed);
  }
  else {
    NRF_LOG_INFO("[SystemTask] Button pushed, waking up");
    displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::GoToRunning);
    isSleeping = false;
    batteryController.Update();
    displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::UpdateBatteryLevel);
  }
}

void SystemTask::OnTouchEvent() {
  NRF_LOG_INFO("[SystemTask] Touch event");
  displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::TouchEvent);
}

void SystemTask::PushMessage(SystemTask::Messages msg) {
  BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(systemTaksMsgQueue, &msg, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken) {
    /* Actual macro used here is port specific. */
    // TODO : should I do something here?
  }
}
