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
#include "drivers/Cst816s.h"
#include "drivers/St7789.h"
#include "drivers/InternalFlash.h"
#include "drivers/SpiMaster.h"
#include "drivers/SpiNorFlash.h"
#include "drivers/TwiMaster.h"
#include "drivers/Hrs3300.h"
#include "main.h"

#include <memory>

using namespace Pinetime::System;

namespace {
  static inline bool in_isr(void) {
    return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;
  }
}

void IdleTimerCallback(TimerHandle_t xTimer) {

  NRF_LOG_INFO("IdleTimerCallback");
  auto sysTask = static_cast<SystemTask*>(pvTimerGetTimerID(xTimer));
  sysTask->OnIdle();
}

SystemTask::SystemTask(Drivers::SpiMaster& spi,
                       Drivers::St7789& lcd,
                       Pinetime::Drivers::SpiNorFlash& spiNorFlash,
                       Drivers::TwiMaster& twiMaster,
                       Drivers::Cst816S& touchPanel,
                       Components::LittleVgl& lvgl,
                       Controllers::Battery& batteryController,
                       Controllers::Ble& bleController,
                       Controllers::DateTime& dateTimeController,
                       Controllers::TimerController& timerController,
                       Drivers::Watchdog& watchdog,
                       Pinetime::Controllers::NotificationManager& notificationManager,
                       Pinetime::Controllers::MotorController& motorController,
                       Pinetime::Drivers::Hrs3300& heartRateSensor,
                       Pinetime::Controllers::MotionController& motionController,
                       Pinetime::Drivers::Bma421& motionSensor,
                       Controllers::Settings& settingsController,
                       Pinetime::Controllers::HeartRateController& heartRateController,
                       Pinetime::Applications::DisplayApp& displayApp,
                       Pinetime::Applications::HeartRateTask& heartRateApp,
                       Pinetime::Controllers::FS& fs)
  : spi {spi},
    lcd {lcd},
    spiNorFlash {spiNorFlash},
    twiMaster {twiMaster},
    touchPanel {touchPanel},
    lvgl {lvgl},
    batteryController {batteryController},
    bleController {bleController},
    dateTimeController {dateTimeController},
    timerController {timerController},
    watchdog {watchdog},
    notificationManager{notificationManager},
    motorController {motorController},
    heartRateSensor {heartRateSensor},
    motionSensor {motionSensor},
    settingsController {settingsController},
    heartRateController{heartRateController},
    motionController{motionController},
    displayApp{displayApp},
    heartRateApp(heartRateApp),
    fs{fs},
    nimbleController(*this, bleController, dateTimeController, notificationManager, batteryController, spiNorFlash, heartRateController) {

}

void SystemTask::Start() {
  systemTasksMsgQueue = xQueueCreate(10, 1);
  if (pdPASS != xTaskCreate(SystemTask::Process, "MAIN", 350, this, 0, &taskHandle))
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
}

void SystemTask::Process(void* instance) {
  auto* app = static_cast<SystemTask*>(instance);
  NRF_LOG_INFO("systemtask task started!");
  app->Work();
}

void SystemTask::Work() {
  watchdog.Setup(7);
  watchdog.Start();
  NRF_LOG_INFO("Last reset reason : %s", Pinetime::Drivers::Watchdog::ResetReasonToString(watchdog.ResetReason()));
  APP_GPIOTE_INIT(2);

  app_timer_init();
  
  spi.Init();
  spiNorFlash.Init();
  spiNorFlash.Wakeup();
  
  fs.Init();

  nimbleController.Init();
  nimbleController.StartAdvertising();
  brightnessController.Init();
  lcd.Init();

  twiMaster.Init();
  touchPanel.Init();
  dateTimeController.Register(this);
  batteryController.Init();
  motorController.Init();
  motionSensor.SoftReset();
  timerController.Register(this);
  timerController.Init();

  // Reset the TWI device because the motion sensor chip most probably crashed it...
  twiMaster.Sleep();
  twiMaster.Init();

  motionSensor.Init();
  motionController.Init(motionSensor.DeviceType());
  settingsController.Init();

  displayApp.Register(this);
  displayApp.Start();

  displayApp.PushMessage(Pinetime::Applications::Display::Messages::UpdateBatteryLevel);

  heartRateSensor.Init();
  heartRateSensor.Disable();
  heartRateApp.Start();

  nrf_gpio_cfg_sense_input(pinButton, (nrf_gpio_pin_pull_t) GPIO_PIN_CNF_PULL_Pulldown, (nrf_gpio_pin_sense_t) GPIO_PIN_CNF_SENSE_High);
  nrf_gpio_cfg_output(15);
  nrf_gpio_pin_set(15);

  nrfx_gpiote_in_config_t pinConfig;
  pinConfig.skip_gpio_setup = true;
  pinConfig.hi_accuracy = false;
  pinConfig.is_watcher = false;
  pinConfig.sense = (nrf_gpiote_polarity_t) NRF_GPIOTE_POLARITY_HITOLO;
  pinConfig.pull = (nrf_gpio_pin_pull_t) GPIO_PIN_CNF_PULL_Pulldown;

  nrfx_gpiote_in_init(pinButton, &pinConfig, nrfx_gpiote_evt_handler);

  nrf_gpio_cfg_sense_input(pinTouchIrq, (nrf_gpio_pin_pull_t) GPIO_PIN_CNF_PULL_Pullup, (nrf_gpio_pin_sense_t) GPIO_PIN_CNF_SENSE_Low);

  pinConfig.skip_gpio_setup = true;
  pinConfig.hi_accuracy = false;
  pinConfig.is_watcher = false;
  pinConfig.sense = (nrf_gpiote_polarity_t) NRF_GPIOTE_POLARITY_HITOLO;
  pinConfig.pull = (nrf_gpio_pin_pull_t) GPIO_PIN_CNF_PULL_Pullup;

  nrfx_gpiote_in_init(pinTouchIrq, &pinConfig, nrfx_gpiote_evt_handler);

  pinConfig.sense = NRF_GPIOTE_POLARITY_TOGGLE;
  pinConfig.pull = NRF_GPIO_PIN_NOPULL;
  pinConfig.is_watcher = false;
  pinConfig.hi_accuracy = false;
  pinConfig.skip_gpio_setup = true;
  nrfx_gpiote_in_init(pinPowerPresentIrq, &pinConfig, nrfx_gpiote_evt_handler);

  if (nrf_gpio_pin_read(pinPowerPresentIrq)) {
    nrf_gpio_cfg_sense_input(pinPowerPresentIrq, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_LOW);
  } else {
    nrf_gpio_cfg_sense_input(pinPowerPresentIrq, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_HIGH);
  }

  idleTimer = xTimerCreate("idleTimer", pdMS_TO_TICKS(settingsController.GetScreenTimeOut()), pdFALSE, this, IdleTimerCallback);
  xTimerStart(idleTimer, 0);

// Suppress endless loop diagnostic
#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
  while (true) {
    UpdateMotion();

    uint8_t msg;
    if (xQueueReceive(systemTasksMsgQueue, &msg, 100)) {

      batteryController.Update();
      // the battery does not emit events when changing charge levels, so we piggyback
      // on any system event to read and update the current values

      Messages message = static_cast<Messages>(msg);
      switch (message) {
        case Messages::EnableSleeping:
          doNotGoToSleep = false;
          break;
        case Messages::DisableSleeping:
          doNotGoToSleep = true;
          break;
        case Messages::UpdateTimeOut:
          xTimerChangePeriod(idleTimer, pdMS_TO_TICKS(settingsController.GetScreenTimeOut()), 0);
          break;
        case Messages::GoToRunning:
          spi.Wakeup();
          twiMaster.Wakeup();

          // Double Tap needs the touch screen to be in normal mode
          if (!settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::DoubleTap)) {
            touchPanel.Wakeup();
          }

          nimbleController.StartAdvertising();
          xTimerStart(idleTimer, 0);
          spiNorFlash.Wakeup();
          lcd.Wakeup();

          displayApp.PushMessage(Pinetime::Applications::Display::Messages::GoToRunning);
          displayApp.PushMessage(Pinetime::Applications::Display::Messages::UpdateBatteryLevel);
          heartRateApp.PushMessage(Pinetime::Applications::HeartRateTask::Messages::WakeUp);

          isSleeping = false;
          isWakingUp = false;
          break;
        case Messages::TouchWakeUp: {
          twiMaster.Wakeup();
          auto touchInfo = touchPanel.GetTouchInfo();
          twiMaster.Sleep();
          if (touchInfo.isTouch and ((touchInfo.gesture == Pinetime::Drivers::Cst816S::Gestures::DoubleTap and
                                      settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::DoubleTap)) or
                                     (touchInfo.gesture == Pinetime::Drivers::Cst816S::Gestures::SingleTap and
                                      settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::SingleTap)))) {
            GoToRunning();
          }
        } break;
        case Messages::GoToSleep:
          isGoingToSleep = true;
          NRF_LOG_INFO("[systemtask] Going to sleep");
          xTimerStop(idleTimer, 0);
          displayApp.PushMessage(Pinetime::Applications::Display::Messages::GoToSleep);
          heartRateApp.PushMessage(Pinetime::Applications::HeartRateTask::Messages::GoToSleep);
          break;
        case Messages::OnNewTime:
          ReloadIdleTimer();
          displayApp.PushMessage(Pinetime::Applications::Display::Messages::UpdateDateTime);
          break;
        case Messages::OnNewNotification:
          if (isSleeping && !isWakingUp) {
            GoToRunning();
          }
          motorController.SetDuration(35);
          displayApp.PushMessage(Pinetime::Applications::Display::Messages::NewNotification);
          break;
        case Messages::OnTimerDone:
          if (isSleeping && !isWakingUp) {
            GoToRunning();
          }
          motorController.SetDuration(35);
          displayApp.PushMessage(Pinetime::Applications::Display::Messages::TimerDone);
          break;
        case Messages::BleConnected:
          ReloadIdleTimer();
          isBleDiscoveryTimerRunning = true;
          bleDiscoveryTimer = 5;
          break;
        case Messages::BleFirmwareUpdateStarted:
          doNotGoToSleep = true;
          if (isSleeping && !isWakingUp)
            GoToRunning();
          displayApp.PushMessage(Pinetime::Applications::Display::Messages::BleFirmwareUpdateStarted);
          break;
        case Messages::BleFirmwareUpdateFinished:
          doNotGoToSleep = false;
          xTimerStart(idleTimer, 0);
          if (bleController.State() == Pinetime::Controllers::Ble::FirmwareUpdateStates::Validated)
            NVIC_SystemReset();
          break;
        case Messages::OnTouchEvent:
          ReloadIdleTimer();
          break;
        case Messages::OnButtonEvent:
          ReloadIdleTimer();
          break;
        case Messages::OnDisplayTaskSleeping:
          if (BootloaderVersion::IsValid()) {
            // First versions of the bootloader do not expose their version and cannot initialize the SPI NOR FLASH
            // if it's in sleep mode. Avoid bricked device by disabling sleep mode on these versions.
            spiNorFlash.Sleep();
          }
          lcd.Sleep();
          spi.Sleep();

          // Double Tap needs the touch screen to be in normal mode
          if (!settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::DoubleTap)) {
            touchPanel.Sleep();
          }
          twiMaster.Sleep();

          isSleeping = true;
          isGoingToSleep = false;
          break;
        case Messages::OnNewDay:
          // We might be sleeping (with TWI device disabled.
          // Remember we'll have to reset the counter next time we're awake
          stepCounterMustBeReset = true;
          break;
        case Messages::OnChargingEvent:
          motorController.SetDuration(15);
	  // Battery level is updated on every message - there's no need to do anything
          break;

        default:
          break;
      }
    }

    if (isBleDiscoveryTimerRunning) {
      if (bleDiscoveryTimer == 0) {
        isBleDiscoveryTimerRunning = false;
        // Services discovery is deffered from 3 seconds to avoid the conflicts between the host communicating with the
        // tharget and vice-versa. I'm not sure if this is the right way to handle this...
        nimbleController.StartDiscovery();
      } else {
        bleDiscoveryTimer--;
      }
    }

    if (xTaskGetTickCount() - batteryNotificationTick > batteryNotificationPeriod) {
      nimbleController.NotifyBatteryLevel(batteryController.PercentRemaining());
      batteryNotificationTick = xTaskGetTickCount();
    }

    monitor.Process();
    uint32_t systick_counter = nrf_rtc_counter_get(portNRF_RTC_REG);
    dateTimeController.UpdateTime(systick_counter);
    if (!nrf_gpio_pin_read(pinButton))
      watchdog.Kick();
  }
// Clear diagnostic suppression
#pragma clang diagnostic pop
}
void SystemTask::UpdateMotion() {
  if (isGoingToSleep or isWakingUp)
    return;

  if (isSleeping && !settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::RaiseWrist))
    return;

  if (isSleeping)
    twiMaster.Wakeup();

  if (stepCounterMustBeReset) {
    motionSensor.ResetStepCounter();
    stepCounterMustBeReset = false;
  }

  auto motionValues = motionSensor.Process();
  if (isSleeping)
    twiMaster.Sleep();

  motionController.IsSensorOk(motionSensor.IsOk());
  motionController.Update(motionValues.x, motionValues.y, motionValues.z, motionValues.steps);
  if (motionController.ShouldWakeUp(isSleeping)) {
    GoToRunning();
  }
}

void SystemTask::OnButtonPushed() {
  if (isGoingToSleep)
    return;
  if (!isSleeping) {
    NRF_LOG_INFO("[systemtask] Button pushed");
    PushMessage(Messages::OnButtonEvent);
    displayApp.PushMessage(Pinetime::Applications::Display::Messages::ButtonPushed);
  } else {
    if (!isWakingUp) {
      NRF_LOG_INFO("[systemtask] Button pushed, waking up");
      GoToRunning();
    }
  }
}

void SystemTask::GoToRunning() {
  if (isGoingToSleep or (not isSleeping) or isWakingUp)
    return;
  isWakingUp = true;
  PushMessage(Messages::GoToRunning);
}

void SystemTask::OnTouchEvent() {
  if (isGoingToSleep)
    return;
  if (!isSleeping) {
    PushMessage(Messages::OnTouchEvent);
    displayApp.PushMessage(Pinetime::Applications::Display::Messages::TouchEvent);
  } else if (!isWakingUp) {
    if (settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::SingleTap) or
        settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::DoubleTap)) {
      PushMessage(Messages::TouchWakeUp);
    }
  }
}

void SystemTask::PushMessage(System::Messages msg) {
  if (msg == Messages::GoToSleep) {
    isGoingToSleep = true;
  }

  if(in_isr()) {
    BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(systemTasksMsgQueue, &msg, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
      /* Actual macro used here is port specific. */
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    }
  } else {
    xQueueSend(systemTasksMsgQueue, &msg, portMAX_DELAY);
  }
}

void SystemTask::OnIdle() {
  if (doNotGoToSleep)
    return;
  NRF_LOG_INFO("Idle timeout -> Going to sleep")
  PushMessage(Messages::GoToSleep);
}

void SystemTask::ReloadIdleTimer() const {
  if (isSleeping || isGoingToSleep)
    return;
  xTimerReset(idleTimer, 0);
}
