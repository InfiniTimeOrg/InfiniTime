#include <libraries/log/nrf_log.h>
#include <libraries/gpiote/app_gpiote.h>
#include <drivers/Cst816s.h>
#include <DisplayApp/LittleVgl.h>
#include <hal/nrf_rtc.h>
#include <Components/Ble/NotificationManager.h>
#include <host/ble_gatt.h>
#include <host/ble_hs_adv.h>
#include "SystemTask.h"
#include <nimble/hci_common.h>
#include <host/ble_gap.h>
#include <host/util/util.h>
#include "../main.h"

using namespace Pinetime::System;

SystemTask::SystemTask(Drivers::SpiMaster &spi, Drivers::St7789 &lcd, Drivers::Cst816S &touchPanel,
                       Components::LittleVgl &lvgl,
                       Controllers::Battery &batteryController, Controllers::Ble &bleController,
                       Controllers::DateTime &dateTimeController,
                       Pinetime::Controllers::NotificationManager& notificationManager) :
                       spi{spi}, lcd{lcd}, touchPanel{touchPanel}, lvgl{lvgl}, batteryController{batteryController},
                       bleController{bleController}, dateTimeController{dateTimeController},
                       watchdog{}, watchdogView{watchdog}, notificationManager{notificationManager},
                       nimbleController(*this, bleController,dateTimeController, notificationManager) {
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

/* BLE */
  nimbleController.Init();
  nimbleController.StartAdvertising();
/* /BLE*/

  spi.Init();
  lcd.Init();
  touchPanel.Init();
  batteryController.Init();

  displayApp.reset(new Pinetime::Applications::DisplayApp(lcd, lvgl, touchPanel, batteryController, bleController,
                                                          dateTimeController, watchdogView, *this, notificationManager));
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
        case Messages::OnNewTime:
          displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::UpdateDateTime);
          break;
        case Messages::OnNewNotification:
          displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::NewNotification);
          break;
        default: break;
      }
    }
    uint32_t systick_counter = nrf_rtc_counter_get(portNRF_RTC_REG);
    dateTimeController.UpdateTime(systick_counter);
    batteryController.Update();

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
