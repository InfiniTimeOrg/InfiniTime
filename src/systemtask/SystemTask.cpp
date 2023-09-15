#include "systemtask/SystemTask.h"
#include <hal/nrf_rtc.h>
#include <libraries/gpiote/app_gpiote.h>
#include <libraries/log/nrf_log.h>
#include "BootloaderVersion.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "displayapp/TouchEvents.h"
#include "drivers/Cst816s.h"
#include "drivers/St7789.h"
#include "drivers/InternalFlash.h"
#include "drivers/SpiMaster.h"
#include "drivers/SpiNorFlash.h"
#include "drivers/TwiMaster.h"
#include "drivers/Hrs3300.h"
#include "drivers/PinMap.h"
#include "main.h"
#include "BootErrors.h"

#include <memory>

using namespace Pinetime::System;

namespace {
  inline bool in_isr() {
    return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;
  }
}

void MeasureBatteryTimerCallback(TimerHandle_t xTimer) {
  auto* sysTask = static_cast<SystemTask*>(pvTimerGetTimerID(xTimer));
  sysTask->PushMessage(Pinetime::System::Messages::MeasureBatteryTimerExpired);
}

SystemTask::SystemTask(Drivers::SpiMaster& spi,
                       Pinetime::Drivers::SpiNorFlash& spiNorFlash,
                       Drivers::TwiMaster& twiMaster,
                       Drivers::Cst816S& touchPanel,
                       Controllers::Battery& batteryController,
                       Controllers::Ble& bleController,
                       Controllers::DateTime& dateTimeController,
                       Controllers::AlarmController& alarmController,
                       Drivers::Watchdog& watchdog,
                       Pinetime::Controllers::NotificationManager& notificationManager,
                       Pinetime::Drivers::Hrs3300& heartRateSensor,
                       Pinetime::Controllers::MotionController& motionController,
                       Pinetime::Drivers::Bma421& motionSensor,
                       Controllers::Settings& settingsController,
                       Pinetime::Controllers::HeartRateController& heartRateController,
                       Pinetime::Applications::DisplayApp& displayApp,
                       Pinetime::Applications::HeartRateTask& heartRateApp,
                       Pinetime::Controllers::FS& fs,
                       Pinetime::Controllers::TouchHandler& touchHandler,
                       Pinetime::Controllers::ButtonHandler& buttonHandler)
  : spi {spi},
    spiNorFlash {spiNorFlash},
    twiMaster {twiMaster},
    touchPanel {touchPanel},
    batteryController {batteryController},
    bleController {bleController},
    dateTimeController {dateTimeController},
    alarmController {alarmController},
    watchdog {watchdog},
    notificationManager {notificationManager},
    heartRateSensor {heartRateSensor},
    motionSensor {motionSensor},
    settingsController {settingsController},
    heartRateController {heartRateController},
    motionController {motionController},
    displayApp {displayApp},
    heartRateApp(heartRateApp),
    fs {fs},
    touchHandler {touchHandler},
    buttonHandler {buttonHandler},
    nimbleController(*this,
                     bleController,
                     dateTimeController,
                     notificationManager,
                     batteryController,
                     spiNorFlash,
                     heartRateController,
                     motionController,
                     fs) {
}

void SystemTask::Start() {
  systemTasksMsgQueue = xQueueCreate(10, 1);
  if (pdPASS != xTaskCreate(SystemTask::Process, "MAIN", 350, this, 1, &taskHandle)) {
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
  }
}

void SystemTask::Process(void* instance) {
  auto* app = static_cast<SystemTask*>(instance);
  NRF_LOG_INFO("systemtask task started!");
  app->Work();
}

void SystemTask::Work() {
  BootErrors bootError = BootErrors::None;

  watchdog.Setup(7, Drivers::Watchdog::SleepBehaviour::Run, Drivers::Watchdog::HaltBehaviour::Pause);
  watchdog.Start();
  NRF_LOG_INFO("Last reset reason : %s", Pinetime::Drivers::ResetReasonToString(watchdog.GetResetReason()));
  APP_GPIOTE_INIT(2);

  spi.Init();
  spiNorFlash.Init();
  spiNorFlash.Wakeup();

  fs.Init();

  nimbleController.Init();

  twiMaster.Init();
  /*
   * TODO We disable this warning message until we ensure it won't be displayed
   * on legitimate PineTime equipped with a compatible touch controller.
   * (some users reported false positive). See https://github.com/InfiniTimeOrg/InfiniTime/issues/763
  if (!touchPanel.Init()) {
    bootError = BootErrors::TouchController;
  }
   */
  touchPanel.Init();
  dateTimeController.Register(this);
  batteryController.Register(this);
  motionSensor.SoftReset();
  alarmController.Init(this);

  // Reset the TWI device because the motion sensor chip most probably crashed it...
  twiMaster.Sleep();
  twiMaster.Init();

  motionSensor.Init();
  motionController.Init(motionSensor.DeviceType());
  settingsController.Init();

  displayApp.Register(this);
  displayApp.Start(bootError);

  heartRateSensor.Init();
  heartRateSensor.Disable();
  heartRateApp.Start();

  buttonHandler.Init(this);

  // Setup Interrupts
  nrfx_gpiote_in_config_t pinConfig;
  pinConfig.skip_gpio_setup = false;
  pinConfig.hi_accuracy = false;
  pinConfig.is_watcher = false;

  // Button
  nrf_gpio_cfg_output(PinMap::ButtonEnable);
  nrf_gpio_pin_set(PinMap::ButtonEnable);
  pinConfig.sense = NRF_GPIOTE_POLARITY_TOGGLE;
  pinConfig.pull = NRF_GPIO_PIN_PULLDOWN;
  nrfx_gpiote_in_init(PinMap::Button, &pinConfig, nrfx_gpiote_evt_handler);
  nrfx_gpiote_in_event_enable(PinMap::Button, true);

  // Touchscreen
  pinConfig.sense = NRF_GPIOTE_POLARITY_HITOLO;
  pinConfig.pull = NRF_GPIO_PIN_PULLUP;
  nrfx_gpiote_in_init(PinMap::Cst816sIrq, &pinConfig, nrfx_gpiote_evt_handler);
  nrfx_gpiote_in_event_enable(PinMap::Cst816sIrq, true);

  // Power present
  pinConfig.sense = NRF_GPIOTE_POLARITY_TOGGLE;
  pinConfig.pull = NRF_GPIO_PIN_NOPULL;
  nrfx_gpiote_in_init(PinMap::PowerPresent, &pinConfig, nrfx_gpiote_evt_handler);
  nrfx_gpiote_in_event_enable(PinMap::PowerPresent, true);

  batteryController.MeasureVoltage();

  measureBatteryTimer = xTimerCreate("measureBattery", batteryMeasurementPeriod, pdTRUE, this, MeasureBatteryTimerCallback);
  xTimerStart(measureBatteryTimer, portMAX_DELAY);

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
  while (true) {
    UpdateMotion();

    Messages msg;
    if (xQueueReceive(systemTasksMsgQueue, &msg, 100) == pdTRUE) {
      switch (msg) {
        case Messages::EnableSleeping:
          // Make sure that exiting an app doesn't enable sleeping,
          // if the exiting was caused by a firmware update
          if (!bleController.IsFirmwareUpdating()) {
            doNotGoToSleep = false;
          }
          displayApp.PushMessage(Pinetime::Applications::Display::Messages::RestoreBrightness);
          break;
        case Messages::DisableSleeping:
          doNotGoToSleep = true;
          break;
        case Messages::GoToRunning:
          spi.Wakeup();

          // Double Tap needs the touch screen to be in normal mode
          if (!settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::DoubleTap)) {
            touchPanel.Wakeup();
          }

          spiNorFlash.Wakeup();

          displayApp.PushMessage(Pinetime::Applications::Display::Messages::GoToRunning);
          heartRateApp.PushMessage(Pinetime::Applications::HeartRateTask::Messages::WakeUp);

          if (bleController.IsRadioEnabled() && !bleController.IsConnected()) {
            nimbleController.RestartFastAdv();
          }

          state = SystemTaskState::Running;
          break;
        case Messages::TouchWakeUp: {
          if (touchHandler.ProcessTouchInfo(touchPanel.GetTouchInfo())) {
            auto gesture = touchHandler.GestureGet();
            if (settingsController.GetNotificationStatus() != Controllers::Settings::Notification::Sleep &&
                gesture != Pinetime::Applications::TouchEvents::None &&
                ((gesture == Pinetime::Applications::TouchEvents::DoubleTap &&
                  settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::DoubleTap)) ||
                 (gesture == Pinetime::Applications::TouchEvents::Tap &&
                  settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::SingleTap)))) {
              GoToRunning();
            }
          }
          break;
        }
        case Messages::GoToSleep:
          if (doNotGoToSleep) {
            break;
          }
          state = SystemTaskState::GoingToSleep; // Already set in PushMessage()
          NRF_LOG_INFO("[systemtask] Going to sleep");
          displayApp.PushMessage(Pinetime::Applications::Display::Messages::GoToSleep);
          heartRateApp.PushMessage(Pinetime::Applications::HeartRateTask::Messages::GoToSleep);
          break;
        case Messages::OnNewTime:
          displayApp.PushMessage(Pinetime::Applications::Display::Messages::RestoreBrightness);
          displayApp.PushMessage(Pinetime::Applications::Display::Messages::UpdateDateTime);
          if (alarmController.State() == Controllers::AlarmController::AlarmState::Set) {
            alarmController.ScheduleAlarm();
          }
          break;
        case Messages::OnNewNotification:
          if (settingsController.GetNotificationStatus() == Pinetime::Controllers::Settings::Notification::On) {
            if (state == SystemTaskState::Sleeping) {
              GoToRunning();
            } else {
              displayApp.PushMessage(Pinetime::Applications::Display::Messages::RestoreBrightness);
            }
            displayApp.PushMessage(Pinetime::Applications::Display::Messages::NewNotification);
          }
          break;
        case Messages::SetOffAlarm:
          if (state == SystemTaskState::Sleeping) {
            GoToRunning();
          }
          displayApp.PushMessage(Pinetime::Applications::Display::Messages::AlarmTriggered);
          break;
        case Messages::BleConnected:
          displayApp.PushMessage(Pinetime::Applications::Display::Messages::RestoreBrightness);
          isBleDiscoveryTimerRunning = true;
          bleDiscoveryTimer = 5;
          break;
        case Messages::BleFirmwareUpdateStarted:
          doNotGoToSleep = true;
          if (state == SystemTaskState::Sleeping) {
            GoToRunning();
          }
          displayApp.PushMessage(Pinetime::Applications::Display::Messages::BleFirmwareUpdateStarted);
          break;
        case Messages::BleFirmwareUpdateFinished:
          if (bleController.State() == Pinetime::Controllers::Ble::FirmwareUpdateStates::Validated) {
            NVIC_SystemReset();
          }
          doNotGoToSleep = false;
          break;
        case Messages::StartFileTransfer:
          NRF_LOG_INFO("[systemtask] FS Started");
          doNotGoToSleep = true;
          if (state == SystemTaskState::Sleeping) {
            GoToRunning();
          }
          // TODO add intent of fs access icon or something
          break;
        case Messages::StopFileTransfer:
          NRF_LOG_INFO("[systemtask] FS Stopped");
          doNotGoToSleep = false;
          // TODO add intent of fs access icon or something
          break;
        case Messages::OnTouchEvent:
          if (touchHandler.ProcessTouchInfo(touchPanel.GetTouchInfo())) {
            displayApp.PushMessage(Pinetime::Applications::Display::Messages::TouchEvent);
          }
          break;
        case Messages::HandleButtonEvent: {
          Controllers::ButtonActions action = Controllers::ButtonActions::None;
          if (nrf_gpio_pin_read(Pinetime::PinMap::Button) == 0) {
            action = buttonHandler.HandleEvent(Controllers::ButtonHandler::Events::Release);
          } else {
            action = buttonHandler.HandleEvent(Controllers::ButtonHandler::Events::Press);
            // This is for faster wakeup, sacrificing special longpress and doubleclick handling while sleeping
            if (IsSleeping()) {
              fastWakeUpDone = true;
              GoToRunning();
              break;
            }
          }
          HandleButtonAction(action);
        } break;
        case Messages::HandleButtonTimerEvent: {
          auto action = buttonHandler.HandleEvent(Controllers::ButtonHandler::Events::Timer);
          HandleButtonAction(action);
        } break;
        case Messages::OnDisplayTaskSleeping:
          if (BootloaderVersion::IsValid()) {
            // First versions of the bootloader do not expose their version and cannot initialize the SPI NOR FLASH
            // if it's in sleep mode. Avoid bricked device by disabling sleep mode on these versions.
            spiNorFlash.Sleep();
          }
          spi.Sleep();

          // Double Tap needs the touch screen to be in normal mode
          if (!settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::DoubleTap)) {
            touchPanel.Sleep();
          }

          state = SystemTaskState::Sleeping;
          break;
        case Messages::OnNewDay:
          // We might be sleeping (with TWI device disabled.
          // Remember we'll have to reset the counter next time we're awake
          stepCounterMustBeReset = true;
          break;
        case Messages::OnNewHour:
          using Pinetime::Controllers::AlarmController;
          if (settingsController.GetNotificationStatus() != Controllers::Settings::Notification::Sleep &&
              settingsController.GetChimeOption() == Controllers::Settings::ChimesOption::Hours &&
              alarmController.State() != AlarmController::AlarmState::Alerting) {
            if (state == SystemTaskState::Sleeping) {
              GoToRunning();
              displayApp.PushMessage(Pinetime::Applications::Display::Messages::Chime);
            }
          }
          break;
        case Messages::OnNewHalfHour:
          using Pinetime::Controllers::AlarmController;
          if (settingsController.GetNotificationStatus() != Controllers::Settings::Notification::Sleep &&
              settingsController.GetChimeOption() == Controllers::Settings::ChimesOption::HalfHours &&
              alarmController.State() != AlarmController::AlarmState::Alerting) {
            if (state == SystemTaskState::Sleeping) {
              GoToRunning();
              displayApp.PushMessage(Pinetime::Applications::Display::Messages::Chime);
            }
          }
          break;
        case Messages::OnChargingEvent:
          batteryController.ReadPowerState();
          displayApp.PushMessage(Applications::Display::Messages::OnChargingEvent);
          if (state == SystemTaskState::Sleeping) {
            GoToRunning();
          }
          break;
        case Messages::MeasureBatteryTimerExpired:
          batteryController.MeasureVoltage();
          break;
        case Messages::BatteryPercentageUpdated:
          nimbleController.NotifyBatteryLevel(batteryController.PercentRemaining());
          break;
        case Messages::OnPairing:
          if (state == SystemTaskState::Sleeping) {
            GoToRunning();
          }
          displayApp.PushMessage(Pinetime::Applications::Display::Messages::ShowPairingKey);
          break;
        case Messages::BleRadioEnableToggle:
          if (settingsController.GetBleRadioEnabled()) {
            nimbleController.EnableRadio();
          } else {
            nimbleController.DisableRadio();
          }
          break;
        default:
          break;
      }
    }

    if (isBleDiscoveryTimerRunning) {
      if (bleDiscoveryTimer == 0) {
        isBleDiscoveryTimerRunning = false;
        // Services discovery is deferred from 3 seconds to avoid the conflicts between the host communicating with the
        // target and vice-versa. I'm not sure if this is the right way to handle this...
        nimbleController.StartDiscovery();
      } else {
        bleDiscoveryTimer--;
      }
    }

    monitor.Process();
    uint32_t systick_counter = nrf_rtc_counter_get(portNRF_RTC_REG);
    dateTimeController.UpdateTime(systick_counter);
    NoInit_BackUpTime = dateTimeController.CurrentDateTime();
    if (nrf_gpio_pin_read(PinMap::Button) == 0) {
      watchdog.Reload();
    }
  }
#pragma clang diagnostic pop
}

void SystemTask::UpdateMotion() {
  if (state == SystemTaskState::GoingToSleep || state == SystemTaskState::WakingUp) {
    return;
  }

  if (state == SystemTaskState::Sleeping && !(settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::RaiseWrist) ||
                                              settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::Shake))) {
    return;
  }

  if (stepCounterMustBeReset) {
    motionSensor.ResetStepCounter();
    stepCounterMustBeReset = false;
  }

  auto motionValues = motionSensor.Process();

  motionController.Update(motionValues.x, motionValues.y, motionValues.z, motionValues.steps);

  if (settingsController.GetNotificationStatus() != Controllers::Settings::Notification::Sleep) {
    if ((settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::RaiseWrist) &&
         motionController.ShouldRaiseWake()) ||
        (settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::Shake) &&
         motionController.ShouldShakeWake(settingsController.GetShakeThreshold()))) {
      GoToRunning();
    }
  }
  if (settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::LowerWrist) && state == SystemTaskState::Running &&
      motionController.ShouldLowerSleep()) {
    PushMessage(Messages::GoToSleep);
  }
}

void SystemTask::HandleButtonAction(Controllers::ButtonActions action) {
  if (IsSleeping()) {
    return;
  }

  displayApp.PushMessage(Pinetime::Applications::Display::Messages::RestoreBrightness);

  using Actions = Controllers::ButtonActions;

  switch (action) {
    case Actions::Click:
      // If the first action after fast wakeup is a click, it should be ignored.
      if (!fastWakeUpDone && state != SystemTaskState::GoingToSleep) {
        displayApp.PushMessage(Applications::Display::Messages::ButtonPushed);
      }
      break;
    case Actions::DoubleClick:
      displayApp.PushMessage(Applications::Display::Messages::ButtonDoubleClicked);
      break;
    case Actions::LongPress:
      displayApp.PushMessage(Applications::Display::Messages::ButtonLongPressed);
      break;
    case Actions::LongerPress:
      displayApp.PushMessage(Applications::Display::Messages::ButtonLongerPressed);
      break;
    default:
      return;
  }

  fastWakeUpDone = false;
}

void SystemTask::GoToRunning() {
  if (state == SystemTaskState::Sleeping) {
    state = SystemTaskState::WakingUp;
    PushMessage(Messages::GoToRunning);
  }
}

void SystemTask::OnTouchEvent() {
  if (state == SystemTaskState::Running) {
    PushMessage(Messages::OnTouchEvent);
  } else if (state == SystemTaskState::Sleeping) {
    if (settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::SingleTap) or
        settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::DoubleTap)) {
      PushMessage(Messages::TouchWakeUp);
    }
  }
}

void SystemTask::PushMessage(System::Messages msg) {
  if (msg == Messages::GoToSleep && !doNotGoToSleep) {
    state = SystemTaskState::GoingToSleep;
  }

  if (in_isr()) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(systemTasksMsgQueue, &msg, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE) {
      /* Actual macro used here is port specific. */
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
  } else {
    xQueueSend(systemTasksMsgQueue, &msg, portMAX_DELAY);
  }
}
