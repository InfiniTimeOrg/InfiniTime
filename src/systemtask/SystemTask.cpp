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
                       Controllers::StopWatchController& stopWatchController,
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
    stopWatchController {stopWatchController},
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
  if (!nrfx_gpiote_is_init()) {
    nrfx_gpiote_init();
  }

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
  displayApp.Register(&nimbleController.weather());
  displayApp.Register(&nimbleController.music());
  displayApp.Register(&nimbleController.navigation());
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

  constexpr TickType_t stateUpdatePeriod = pdMS_TO_TICKS(100);
  // Stores when the state (motion, watchdog, time persistence etc) was last updated
  // If there are many events being received by the message queue, this prevents
  // having to update motion etc after every single event, which is bad
  // for efficiency and for motion wake algorithms which expect motion readings
  // to be 100ms apart
  TickType_t lastStateUpdate = xTaskGetTickCount() - stateUpdatePeriod; // Force immediate run
  TickType_t elapsed;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
  while (true) {
    Messages msg;

    elapsed = xTaskGetTickCount() - lastStateUpdate;
    TickType_t waitTime;
    if (elapsed >= stateUpdatePeriod) {
      waitTime = 0;
    } else {
      waitTime = stateUpdatePeriod - elapsed;
    }
    if (xQueueReceive(systemTasksMsgQueue, &msg, waitTime) == pdTRUE) {
      switch (msg) {
        case Messages::EnableSleeping:
          wakeLocksHeld--;
          break;
        case Messages::DisableSleeping:
          GoToRunning();
          wakeLocksHeld++;
          break;
        case Messages::GoToRunning:
          GoToRunning();
          break;
        case Messages::GoToSleep:
          GoToSleep();
          break;
        case Messages::OnNewTime:
          if (alarmController.IsEnabled()) {
            alarmController.ScheduleAlarm();
          }
          break;
        case Messages::OnNewNotification:
          if (settingsController.GetNotificationStatus() == Pinetime::Controllers::Settings::Notification::On) {
            if (IsSleeping()) {
              GoToRunning();
            }
            displayApp.PushMessage(Pinetime::Applications::Display::Messages::NewNotification);
          }
          break;
        case Messages::SetOffAlarm:
          GoToRunning();
          displayApp.PushMessage(Pinetime::Applications::Display::Messages::AlarmTriggered);
          break;
        case Messages::BleConnected:
          displayApp.PushMessage(Pinetime::Applications::Display::Messages::NotifyDeviceActivity);
          isBleDiscoveryTimerRunning = true;
          bleDiscoveryTimer = 5;
          break;
        case Messages::BleFirmwareUpdateStarted:
          GoToRunning();
          wakeLocksHeld++;
          displayApp.PushMessage(Pinetime::Applications::Display::Messages::BleFirmwareUpdateStarted);
          break;
        case Messages::BleFirmwareUpdateFinished:
          if (bleController.State() == Pinetime::Controllers::Ble::FirmwareUpdateStates::Validated) {
            NVIC_SystemReset();
          }
          wakeLocksHeld--;
          break;
        case Messages::StartFileTransfer:
          NRF_LOG_INFO("[systemtask] FS Started");
          GoToRunning();
          wakeLocksHeld++;
          // TODO add intent of fs access icon or something
          break;
        case Messages::StopFileTransfer:
          NRF_LOG_INFO("[systemtask] FS Stopped");
          wakeLocksHeld--;
          // TODO add intent of fs access icon or something
          break;
        case Messages::OnTouchEvent:
          // Finish immediately if no new events
          if (!touchHandler.ProcessTouchInfo(touchPanel.GetTouchInfo())) {
            break;
          }
          if (state == SystemTaskState::Running) {
            displayApp.PushMessage(Pinetime::Applications::Display::Messages::TouchEvent);
          } else {
            // If asleep, check for touch panel wake triggers
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
        case Messages::OnDisplayTaskAOD:
          // The state was set to GoingToSleep when GoToSleep() was called
          // If the state is no longer GoingToSleep, we have since transitioned back to Running
          // In this case absorb the OnDisplayTaskSleeping/AOD
          // as DisplayApp is about to receive GoToRunning
          if (state != SystemTaskState::GoingToSleep) {
            break;
          }
          if (BootloaderVersion::IsValid()) {
            // First versions of the bootloader do not expose their version and cannot initialize the SPI NOR FLASH
            // if it's in sleep mode. Avoid bricked device by disabling sleep mode on these versions.
            spiNorFlash.Sleep();
          }

          // Must keep SPI awake when still updating the display for always on
          if (msg == Messages::OnDisplayTaskSleeping) {
            spi.Sleep();
          }

          // Double Tap needs the touch screen to be in normal mode
          if (!settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::DoubleTap)) {
            touchPanel.Sleep();
          }

          if (msg == Messages::OnDisplayTaskSleeping) {
            state = SystemTaskState::Sleeping;
          } else {
            state = SystemTaskState::AODSleeping;
          }
          break;
        case Messages::OnNewDay:
          motionSensor.ResetStepCounter();
          motionController.AdvanceDay();
          break;
        case Messages::OnNewHour:
          using Pinetime::Controllers::AlarmController;
          if (settingsController.GetNotificationStatus() != Controllers::Settings::Notification::Sleep &&
              settingsController.GetChimeOption() == Controllers::Settings::ChimesOption::Hours && !alarmController.IsAlerting()) {
            GoToRunning();
            displayApp.PushMessage(Pinetime::Applications::Display::Messages::Chime);
          }
          break;
        case Messages::OnNewHalfHour:
          using Pinetime::Controllers::AlarmController;
          if (settingsController.GetNotificationStatus() != Controllers::Settings::Notification::Sleep &&
              settingsController.GetChimeOption() == Controllers::Settings::ChimesOption::HalfHours && !alarmController.IsAlerting()) {
            GoToRunning();
            displayApp.PushMessage(Pinetime::Applications::Display::Messages::Chime);
          }
          break;
        case Messages::OnChargingEvent:
          batteryController.ReadPowerState();
          GoToRunning();
          break;
        case Messages::MeasureBatteryTimerExpired:
          batteryController.MeasureVoltage();
          break;
        case Messages::BatteryPercentageUpdated:
          nimbleController.NotifyBatteryLevel(batteryController.PercentRemaining());
          break;
        case Messages::OnPairing:
          GoToRunning();
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
    elapsed = xTaskGetTickCount() - lastStateUpdate;
    if (elapsed >= stateUpdatePeriod) {
      UpdateMotion();
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
      NoInit_BackUpTime = dateTimeController.CurrentDateTime();
      if (nrf_gpio_pin_read(PinMap::Button) == 0) {
        watchdog.Reload();
      }
      lastStateUpdate = xTaskGetTickCount();
    }
  }
#pragma clang diagnostic pop
}

void SystemTask::GoToRunning() {
  if (state == SystemTaskState::Running) {
    return;
  }
  if (state == SystemTaskState::Sleeping || state == SystemTaskState::AODSleeping) {
    // SPI only switched off when entering Sleeping, not AOD or GoingToSleep
    if (state == SystemTaskState::Sleeping) {
      spi.Wakeup();
    }

    // Double Tap needs the touch screen to be in normal mode
    if (!settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::DoubleTap)) {
      touchPanel.Wakeup();
    }

    spiNorFlash.Wakeup();
  }

  displayApp.PushMessage(Pinetime::Applications::Display::Messages::GoToRunning);
  heartRateApp.PushMessage(Pinetime::Applications::HeartRateTask::Messages::WakeUp);

  if (bleController.IsRadioEnabled() && !bleController.IsConnected()) {
    nimbleController.RestartFastAdv();
  }

  state = SystemTaskState::Running;
};

void SystemTask::GoToSleep() {
  if (IsSleeping()) {
    return;
  }
  if (IsSleepDisabled()) {
    return;
  }
  NRF_LOG_INFO("[systemtask] Going to sleep");
  if (settingsController.GetAlwaysOnDisplay()) {
    displayApp.PushMessage(Pinetime::Applications::Display::Messages::GoToAOD);
  } else {
    displayApp.PushMessage(Pinetime::Applications::Display::Messages::GoToSleep);
  }
  heartRateApp.PushMessage(Pinetime::Applications::HeartRateTask::Messages::GoToSleep);

  state = SystemTaskState::GoingToSleep;
};

void SystemTask::UpdateMotion() {
  // Unconditionally update motion
  // Reading steps/motion characteristics must return up to date information even when not subscribed to notifications

  auto motionValues = motionSensor.Process();

  motionController.Update(motionValues.x, motionValues.y, motionValues.z, motionValues.steps);

  if (settingsController.GetNotificationStatus() != Controllers::Settings::Notification::Sleep) {
    if ((settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::RaiseWrist) &&
         motionController.ShouldRaiseWake()) ||
        (settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::Shake) &&
         motionController.CurrentShakeSpeed() > settingsController.GetShakeThreshold())) {
      GoToRunning();
    }
  }
  if (settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::LowerWrist) && state == SystemTaskState::Running &&
      motionController.ShouldLowerSleep()) {
    GoToSleep();
  }
}

void SystemTask::HandleButtonAction(Controllers::ButtonActions action) {
  if (IsSleeping()) {
    return;
  }

  displayApp.PushMessage(Pinetime::Applications::Display::Messages::NotifyDeviceActivity);

  using Actions = Controllers::ButtonActions;

  switch (action) {
    case Actions::Click:
      // If the first action after fast wakeup is a click, it should be ignored.
      if (!fastWakeUpDone) {
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

void SystemTask::PushMessage(System::Messages msg) {
  if (in_isr()) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(systemTasksMsgQueue, &msg, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  } else {
    xQueueSend(systemTasksMsgQueue, &msg, portMAX_DELAY);
  }
}
