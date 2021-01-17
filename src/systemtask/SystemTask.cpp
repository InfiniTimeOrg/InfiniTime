#include "SystemTask.h"
#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#include <host/ble_gatt.h>
#include <host/ble_hs_adv.h>
#include <host/util/util.h>
#include <nimble/hci_common.h>
#undef max
#undef min
#include <hal/nrf_rtc.h>
#include <libraries/gpiote/app_gpiote.h>
#include <libraries/log/nrf_log.h>

#include "BootloaderVersion.h"
#include "components/ble/BleController.h"
#include "displayapp/LittleVgl.h"
#include "drivers/Cst816s.h"
#include "drivers/St7789.h"
#include "drivers/InternalFlash.h"
#include "drivers/SpiMaster.h"
#include "drivers/SpiNorFlash.h"
#include "drivers/TwiMaster.h"
#include "drivers/Hrs3300.h"
#include "main.h"

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
                       Pinetime::Controllers::NotificationManager& notificationManager,
                       Pinetime::Drivers::Hrs3300& heartRateSensor) :
                       spi{spi}, lcd{lcd}, spiNorFlash{spiNorFlash},
                       twiMaster{twiMaster}, touchPanel{touchPanel}, lvgl{lvgl}, batteryController{batteryController},
                       heartRateController{*this},
                       bleController{bleController}, dateTimeController{dateTimeController},
                       watchdog{}, watchdogView{watchdog}, notificationManager{notificationManager},
                       heartRateSensor{heartRateSensor},
                       nimbleController(*this, bleController,dateTimeController, notificationManager, batteryController, spiNorFlash, heartRateController) {
  systemTasksMsgQueue = xQueueCreate(10, 1);
}

void SystemTask::Start() {
  if (pdPASS != xTaskCreate(SystemTask::Process, "MAIN", 350, this, 0, &taskHandle))
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
}

void SystemTask::Process(void *instance) {
  auto *app = static_cast<SystemTask *>(instance);
  NRF_LOG_INFO("systemtask task started!");
  app->Work();
}

void SystemTask::Work() {
  watchdog.Setup(7);
  watchdog.Start();
  NRF_LOG_INFO("Last reset reason : %s", Pinetime::Drivers::Watchdog::ResetReasonToString(watchdog.ResetReason()));
  APP_GPIOTE_INIT(2);

  spi.Init();
  spiNorFlash.Init();
  spiNorFlash.Wakeup();
  nimbleController.Init();
  nimbleController.StartAdvertising();
  lcd.Init();

  twiMaster.Init();
  touchPanel.Init();
  batteryController.Init();

  displayApp.reset(new Pinetime::Applications::DisplayApp(lcd, lvgl, touchPanel, batteryController, bleController,
                                                          dateTimeController, watchdogView, *this, notificationManager, heartRateController));
  displayApp->Start();

  batteryController.Update();
  displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::UpdateBatteryLevel);


  heartRateSensor.Init();
  heartRateSensor.Disable();
  heartRateApp.reset(new Pinetime::Applications::HeartRateTask(heartRateSensor, heartRateController));
  heartRateApp->Start();


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

  // Suppress endless loop diagnostic
  #pragma clang diagnostic push
  #pragma ide diagnostic ignored "EndlessLoop"
  while(true) {
    uint8_t msg;
    if (xQueueReceive(systemTasksMsgQueue, &msg, isSleeping ? 2500 : 1000)) {
      batteryController.Update();
      Messages message = static_cast<Messages >(msg);
      switch(message) {
        case Messages::GoToRunning:
          spi.Wakeup();
          twiMaster.Wakeup();

          nimbleController.StartAdvertising();
          xTimerStart(idleTimer, 0);
          spiNorFlash.Wakeup();
          touchPanel.Wakeup();
          lcd.Wakeup();

          displayApp->PushMessage(Applications::DisplayApp::Messages::GoToRunning);
          displayApp->PushMessage(Applications::DisplayApp::Messages::UpdateBatteryLevel);
          heartRateApp->PushMessage(Pinetime::Applications::HeartRateTask::Messages::WakeUp);

          isSleeping = false;
          isWakingUp = false;
          break;
        case Messages::GoToSleep:
          isGoingToSleep = true;
          NRF_LOG_INFO("[systemtask] Going to sleep");
          xTimerStop(idleTimer, 0);
          displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::GoToSleep);
          heartRateApp->PushMessage(Pinetime::Applications::HeartRateTask::Messages::GoToSleep);
          break;
        case Messages::OnNewTime:
          ReloadIdleTimer();
          displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::UpdateDateTime);
          break;
        case Messages::OnNewNotification:
          if(isSleeping && !isWakingUp) GoToRunning();
          displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::NewNotification);
          break;
        case Messages::BleConnected:
          ReloadIdleTimer();
          isBleDiscoveryTimerRunning = true;
          bleDiscoveryTimer = 5;
          break;
        case Messages::BleFirmwareUpdateStarted:
          doNotGoToSleep = true;
          if(isSleeping && !isWakingUp) GoToRunning();
          displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::BleFirmwareUpdateStarted);
          break;
        case Messages::BleFirmwareUpdateFinished:
          doNotGoToSleep = false;
          xTimerStart(idleTimer, 0);
          if(bleController.State() == Pinetime::Controllers::Ble::FirmwareUpdateStates::Validated)
            NVIC_SystemReset();
          break;
        case Messages::OnTouchEvent:
          ReloadIdleTimer();
          break;
        case Messages::OnButtonEvent:
          ReloadIdleTimer();
          break;
        case Messages::OnDisplayTaskSleeping:
          if(BootloaderVersion::IsValid()) {
            // First versions of the bootloader do not expose their version and cannot initialize the SPI NOR FLASH
            // if it's in sleep mode. Avoid bricked device by disabling sleep mode on these versions.
            spiNorFlash.Sleep();
          }
          lcd.Sleep();
          touchPanel.Sleep();

          spi.Sleep();
          twiMaster.Sleep();
          isSleeping = true;
          isGoingToSleep = false;
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

    monitor.Process();
    uint32_t systick_counter = nrf_rtc_counter_get(portNRF_RTC_REG);
    dateTimeController.UpdateTime(systick_counter);
    if(!nrf_gpio_pin_read(pinButton))
      watchdog.Kick();
  }
  // Clear diagnostic suppression
  #pragma clang diagnostic pop
}

void SystemTask::OnButtonPushed() {
  if(isGoingToSleep) return;
  if(!isSleeping) {
    NRF_LOG_INFO("[systemtask] Button pushed");
    PushMessage(Messages::OnButtonEvent);
    displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::ButtonPushed);
  }
  else {
    if(!isWakingUp) {
      NRF_LOG_INFO("[systemtask] Button pushed, waking up");
      GoToRunning();
    }
  }
}

void SystemTask::GoToRunning() {
  isWakingUp = true;
  PushMessage(Messages::GoToRunning);
}

void SystemTask::OnTouchEvent() {
  if(isGoingToSleep) return ;
  NRF_LOG_INFO("[systemtask] Touch event");
  if(!isSleeping) {
    PushMessage(Messages::OnTouchEvent);
    displayApp->PushMessage(Pinetime::Applications::DisplayApp::Messages::TouchEvent);
  }
}

void SystemTask::PushMessage(SystemTask::Messages msg) {
  if(msg == Messages::GoToSleep) {
    isGoingToSleep = true;
  }
  BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;
  xQueueSendFromISR(systemTasksMsgQueue, &msg, &xHigherPriorityTaskWoken);
  if (xHigherPriorityTaskWoken) {
    /* Actual macro used here is port specific. */
    // TODO: should I do something here?
  }
}

void SystemTask::OnIdle() {
  if(doNotGoToSleep) return;
  NRF_LOG_INFO("Idle timeout -> Going to sleep")
  PushMessage(Messages::GoToSleep);
}

void SystemTask::ReloadIdleTimer() const {
  if(isSleeping || isGoingToSleep) return;
  xTimerReset(idleTimer, 0);
}
