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
#include <drivers/InternalFlash.h>
#include "../main.h"

using namespace Pinetime::System;

void IdleTimerCallback(TimerHandle_t xTimer) {

  NRF_LOG_INFO("IdleTimerCallback");
  auto sysTask = static_cast<SystemTask *>(pvTimerGetTimerID(xTimer));
  sysTask->OnIdle();
}


SystemTask::SystemTask(Drivers::SpiMaster &spi, Drivers::St7789 &lcd,
                       Pinetime::Drivers::SpiNorFlash& spiNorFlash,
                       Drivers::TwiMaster& twiMaster, Drivers::Cst816S &touchPanel,
                       Components::LittleVgl &lvgl,
                       Controllers::Battery &batteryController, Controllers::Ble &bleController,
                       Controllers::DateTime &dateTimeController,
                       Pinetime::Controllers::NotificationManager& notificationManager) :
                       spi{spi}, lcd{lcd}, spiNorFlash{spiNorFlash},
                       twiMaster{twiMaster}, touchPanel{touchPanel}, lvgl{lvgl}, batteryController{batteryController},
                       bleController{bleController}, dateTimeController{dateTimeController},
                       watchdog{}, watchdogView{watchdog}, notificationManager{notificationManager},
                       nimbleController(*this, bleController,dateTimeController, notificationManager, spiNorFlash) {
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

  spi.Init();
  spiNorFlash.Init();

  // Write the 'image OK' flag if it's not already done
  // TODO implement a better verification mecanism for the image (ask for user confirmation via UI/BLE ?)
  uint32_t* imageOkPtr = reinterpret_cast<uint32_t *>(0x7BFE8);
  uint32_t imageOk = *imageOkPtr;
  if(imageOk != 1)
    Pinetime::Drivers::InternalFlash::WriteWord(0x7BFE8, 1);

  nimbleController.Init();
  nimbleController.StartAdvertising();
  lcd.Init();

  twiMaster.Init();
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

  idleTimer = xTimerCreate ("idleTimer", idleTime, pdFALSE, this, IdleTimerCallback);
  xTimerStart(idleTimer, 0);

  while(true) {
    uint8_t msg;
    if (xQueueReceive(systemTaksMsgQueue, &msg, isSleeping?2500 : 1000)) {
      Messages message = static_cast<Messages >(msg);
      switch(message) {
        case Messages::GoToRunning:
          isSleeping = false;
          xTimerStart(idleTimer, 0);
          nimbleController.StartAdvertising();
          break;
        case Messages::GoToSleep:
          NRF_LOG_INFO("[SystemTask] Going to sleep");
          xTimerStop(idleTimer, 0);
          displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::GoToSleep);
          isSleeping = true;
          break;
        case Messages::OnNewTime:
          ReloadIdleTimer();
          displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::UpdateDateTime);
          break;
        case Messages::OnNewNotification:
          if(isSleeping) GoToRunning();
          displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::NewNotification);
          break;
        case Messages::BleConnected:
          ReloadIdleTimer();
          isBleDiscoveryTimerRunning = true;
          bleDiscoveryTimer = 5;
          break;
        case Messages::BleFirmwareUpdateStarted:
          doNotGoToSleep = true;
          if(isSleeping) GoToRunning();
          displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::BleFirmwareUpdateStarted);
          break;
        case Messages::BleFirmwareUpdateFinished:
          doNotGoToSleep = false;
          xTimerStart(idleTimer, 0);
          displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::BleFirmwareUpdateFinished);
          if(bleController.State() == Pinetime::Controllers::Ble::FirmwareUpdateStates::Validated)
            NVIC_SystemReset();
          break;
        case Messages::OnTouchEvent:
          ReloadIdleTimer();
          break;
        case Messages::OnButtonEvent:
          ReloadIdleTimer();
          break;
        default: break;
      }
    }

    if(isBleDiscoveryTimerRunning) {
      if(bleDiscoveryTimer == 0) {
        isBleDiscoveryTimerRunning = false;
        // Services discovery is deffered from 3 seconds to avoid the conflicts between the host communicating with the
        // tharget and vice-versa. I'm not sure if this is the right way to handle this...
        nimbleController.StartDiscovery();
      } else {
        bleDiscoveryTimer--;
      }
    }

    uint32_t systick_counter = nrf_rtc_counter_get(portNRF_RTC_REG);
    dateTimeController.UpdateTime(systick_counter);
    batteryController.Update();

    monitor.Process();

    if(!nrf_gpio_pin_read(pinButton))
      watchdog.Kick();
  }
}

void SystemTask::OnButtonPushed() {
  if(!isSleeping) {
    NRF_LOG_INFO("[SystemTask] Button pushed");
    PushMessage(Messages::OnButtonEvent);
    displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::ButtonPushed);
  }
  else {
    NRF_LOG_INFO("[SystemTask] Button pushed, waking up");
    GoToRunning();
  }
}

void SystemTask::GoToRunning() {
  PushMessage(Messages::GoToRunning);
  displayApp->PushMessage(Applications::DisplayApp::Messages::GoToRunning);
  displayApp->PushMessage(Applications::DisplayApp::Messages::UpdateBatteryLevel);
}

void SystemTask::OnTouchEvent() {
  NRF_LOG_INFO("[SystemTask] Touch event");
  if(!isSleeping) {
    PushMessage(Messages::OnTouchEvent);
    displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::TouchEvent);
  }
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

void SystemTask::OnIdle() {
  if(doNotGoToSleep) return;
  NRF_LOG_INFO("Idle timeout -> Going to sleep")
  PushMessage(Messages::GoToSleep);
}

void SystemTask::ReloadIdleTimer() const {
  if(isSleeping) return;
  xTimerReset(idleTimer, 0);
}
