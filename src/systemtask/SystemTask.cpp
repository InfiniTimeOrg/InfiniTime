#include "systemtask/SystemTask.h"
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
#include "drivers/PinMap.h"
#include "main.h"
#include "BootErrors.h"

#include <memory>

using namespace Pinetime::System;

namespace {
  static inline bool in_isr(void) {
    return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;
  }
}

void DimTimerCallback(TimerHandle_t xTimer) {

  NRF_LOG_INFO("DimTimerCallback");
  auto sysTask = static_cast<SystemTask*>(pvTimerGetTimerID(xTimer));
  sysTask->OnDim();
}

void IdleTimerCallback(TimerHandle_t xTimer) {

  NRF_LOG_INFO("IdleTimerCallback");
  auto sysTask = static_cast<SystemTask*>(pvTimerGetTimerID(xTimer));
  sysTask->OnIdle();
}

void MeasureBatteryTimerCallback(TimerHandle_t xTimer) {
  auto* sysTask = static_cast<SystemTask*>(pvTimerGetTimerID(xTimer));
  sysTask->PushMessage(Pinetime::System::Messages::MeasureBatteryTimerExpired);
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
                       Controllers::AlarmController& alarmController,
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
                       Pinetime::Controllers::FS& fs,
                       Pinetime::Controllers::TouchHandler& touchHandler,
                       Pinetime::Controllers::ButtonHandler& buttonHandler)
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
    alarmController {alarmController},
    watchdog {watchdog},
    notificationManager {notificationManager},
    motorController {motorController},
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
  if (pdPASS != xTaskCreate(SystemTask::Process, "MAIN", 350, this, 0, &taskHandle)) {
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
  lcd.Init();

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
  motorController.Init();
  motionSensor.SoftReset();
  timerController.Register(this);
  timerController.Init();
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

  // Button
  nrf_gpio_cfg_output(15);
  nrf_gpio_pin_set(15);

  nrfx_gpiote_in_config_t pinConfig;
  pinConfig.skip_gpio_setup = false;
  pinConfig.hi_accuracy = false;
  pinConfig.is_watcher = false;
  pinConfig.sense = static_cast<nrf_gpiote_polarity_t>(NRF_GPIOTE_POLARITY_TOGGLE);
  pinConfig.pull = static_cast<nrf_gpio_pin_pull_t>(GPIO_PIN_CNF_PULL_Pulldown);

  nrfx_gpiote_in_init(PinMap::Button, &pinConfig, nrfx_gpiote_evt_handler);
  nrfx_gpiote_in_event_enable(PinMap::Button, true);

  // Touchscreen
  nrf_gpio_cfg_sense_input(PinMap::Cst816sIrq,
                           static_cast<nrf_gpio_pin_pull_t>(GPIO_PIN_CNF_PULL_Pullup),
                           static_cast<nrf_gpio_pin_sense_t> GPIO_PIN_CNF_SENSE_Low);

  pinConfig.skip_gpio_setup = true;
  pinConfig.hi_accuracy = false;
  pinConfig.is_watcher = false;
  pinConfig.sense = static_cast<nrf_gpiote_polarity_t>(NRF_GPIOTE_POLARITY_HITOLO);
  pinConfig.pull = static_cast<nrf_gpio_pin_pull_t> GPIO_PIN_CNF_PULL_Pullup;

  nrfx_gpiote_in_init(PinMap::Cst816sIrq, &pinConfig, nrfx_gpiote_evt_handler);

  // Power present
  pinConfig.sense = NRF_GPIOTE_POLARITY_TOGGLE;
  pinConfig.pull = NRF_GPIO_PIN_NOPULL;
  pinConfig.is_watcher = false;
  pinConfig.hi_accuracy = false;
  pinConfig.skip_gpio_setup = false;
  nrfx_gpiote_in_init(PinMap::PowerPresent, &pinConfig, nrfx_gpiote_evt_handler);
  nrfx_gpiote_in_event_enable(PinMap::PowerPresent, true);

  batteryController.MeasureVoltage();

  idleTimer = xTimerCreate("idleTimer", pdMS_TO_TICKS(2000), pdFALSE, this, IdleTimerCallback);
  dimTimer = xTimerCreate("dimTimer", pdMS_TO_TICKS(settingsController.GetScreenTimeOut() - 2000), pdFALSE, this, DimTimerCallback);
  measureBatteryTimer = xTimerCreate("measureBattery", batteryMeasurementPeriod, pdTRUE, this, MeasureBatteryTimerCallback);
  xTimerStart(dimTimer, 0);
  xTimerStart(measureBatteryTimer, portMAX_DELAY);

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"
  while (true) {
    UpdateMotion();

    uint8_t msg;
    if (xQueueReceive(systemTasksMsgQueue, &msg, 100)) {
      Messages message = static_cast<Messages>(msg);
      switch (message) {
        case Messages::EnableSleeping:
          // Make sure that exiting an app doesn't enable sleeping,
          // if the exiting was caused by a firmware update
          if (!bleController.IsFirmwareUpdating()) {
            doNotGoToSleep = false;
          }
          break;
        case Messages::DisableSleeping:
          doNotGoToSleep = true;
          break;
        case Messages::UpdateTimeOut:
          xTimerChangePeriod(dimTimer, pdMS_TO_TICKS(settingsController.GetScreenTimeOut() - 2000), 0);
          break;
        case Messages::GoToRunning:
          spi.Wakeup();

          // Double Tap needs the touch screen to be in normal mode
          if (!settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::DoubleTap)) {
            touchPanel.Wakeup();
          }

          xTimerStart(dimTimer, 0);
          spiNorFlash.Wakeup();
          lcd.Wakeup();

          displayApp.PushMessage(Pinetime::Applications::Display::Messages::GoToRunning);
          heartRateApp.PushMessage(Pinetime::Applications::HeartRateTask::Messages::WakeUp);

          if (!bleController.IsConnected()) {
            nimbleController.RestartFastAdv();
          }

          isSleeping = false;
          isWakingUp = false;
          isDimmed = false;
          break;
        case Messages::TouchWakeUp: {
          if (touchHandler.GetNewTouchInfo()) {
            auto gesture = touchHandler.GestureGet();
            if (gesture != Pinetime::Drivers::Cst816S::Gestures::None and
                ((gesture == Pinetime::Drivers::Cst816S::Gestures::DoubleTap and
                  settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::DoubleTap)) or
                 (gesture == Pinetime::Drivers::Cst816S::Gestures::SingleTap and
                  settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::SingleTap)))) {
              GoToRunning();
            }
          }
        } break;
        case Messages::GoToSleep:
          if (doNotGoToSleep) {
            break;
          }
          isGoingToSleep = true;
          NRF_LOG_INFO("[systemtask] Going to sleep");
          xTimerStop(idleTimer, 0);
          xTimerStop(dimTimer, 0);
          displayApp.PushMessage(Pinetime::Applications::Display::Messages::GoToSleep);
          heartRateApp.PushMessage(Pinetime::Applications::HeartRateTask::Messages::GoToSleep);
          break;
        case Messages::OnNewTime:
          ReloadIdleTimer();
          displayApp.PushMessage(Pinetime::Applications::Display::Messages::UpdateDateTime);
          break;
        case Messages::OnNewNotification:
          if (settingsController.GetNotificationStatus() == Pinetime::Controllers::Settings::Notification::ON) {
            if (isSleeping && !isWakingUp) {
              GoToRunning();
            } else {
              ReloadIdleTimer();
            }
            displayApp.PushMessage(Pinetime::Applications::Display::Messages::NewNotification);
          }
          break;
        case Messages::OnTimerDone:
          if (isSleeping && !isWakingUp) {
            GoToRunning();
          }
          motorController.RunForDuration(35);
          displayApp.PushMessage(Pinetime::Applications::Display::Messages::TimerDone);
          break;
        case Messages::SetOffAlarm:
          if (isSleeping && !isWakingUp) {
            GoToRunning();
          }
          motorController.StartRinging();
          displayApp.PushMessage(Pinetime::Applications::Display::Messages::AlarmTriggered);
          break;
        case Messages::StopRinging:
          motorController.StopRinging();
          break;
        case Messages::BleConnected:
          ReloadIdleTimer();
          isBleDiscoveryTimerRunning = true;
          bleDiscoveryTimer = 5;
          break;
        case Messages::BleFirmwareUpdateStarted:
          doNotGoToSleep = true;
          if (isSleeping && !isWakingUp) {
            GoToRunning();
          }
          displayApp.PushMessage(Pinetime::Applications::Display::Messages::BleFirmwareUpdateStarted);
          break;
        case Messages::BleFirmwareUpdateFinished:
          if (bleController.State() == Pinetime::Controllers::Ble::FirmwareUpdateStates::Validated) {
            NVIC_SystemReset();
          }
          doNotGoToSleep = false;
          xTimerStart(dimTimer, 0);
          break;
        case Messages::StartFileTransfer:
          NRF_LOG_INFO("[systemtask] FS Started");
          doNotGoToSleep = true;
          if (isSleeping && !isWakingUp)
            GoToRunning();
          // TODO add intent of fs access icon or something
          break;
        case Messages::StopFileTransfer:
          NRF_LOG_INFO("[systemtask] FS Stopped");
          doNotGoToSleep = false;
          xTimerStart(dimTimer, 0);
          // TODO add intent of fs access icon or something
          break;
        case Messages::OnTouchEvent:
          if (touchHandler.GetNewTouchInfo()) {
            touchHandler.UpdateLvglTouchPoint();
          }
          ReloadIdleTimer();
          displayApp.PushMessage(Pinetime::Applications::Display::Messages::TouchEvent);
          break;
        case Messages::HandleButtonEvent: {
          Controllers::ButtonActions action;
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
          lcd.Sleep();
          spi.Sleep();

          // Double Tap needs the touch screen to be in normal mode
          if (!settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::DoubleTap)) {
            touchPanel.Sleep();
          }

          isSleeping = true;
          isGoingToSleep = false;
          break;
        case Messages::OnNewDay:
          // We might be sleeping (with TWI device disabled.
          // Remember we'll have to reset the counter next time we're awake
          stepCounterMustBeReset = true;
          break;
        case Messages::OnNewHour:
          using Pinetime::Controllers::AlarmController;
          if (settingsController.GetChimeOption() == Controllers::Settings::ChimesOption::Hours && alarmController.State() != AlarmController::AlarmState::Alerting) {
            if (isSleeping && !isWakingUp) {
              GoToRunning();
              displayApp.PushMessage(Pinetime::Applications::Display::Messages::Clock);
            }
            motorController.RunForDuration(35);
          }
          break;
        case Messages::OnNewHalfHour:
          using Pinetime::Controllers::AlarmController;
          if (settingsController.GetChimeOption() == Controllers::Settings::ChimesOption::HalfHours && alarmController.State() != AlarmController::AlarmState::Alerting) {
            if (isSleeping && !isWakingUp) {
              GoToRunning();
              displayApp.PushMessage(Pinetime::Applications::Display::Messages::Clock);
            }
            motorController.RunForDuration(35);
          }
          break;
        case Messages::OnChargingEvent:
          batteryController.ReadPowerState();
          motorController.RunForDuration(15);
          ReloadIdleTimer();
          if (isSleeping && !isWakingUp) {
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
          if (isSleeping && !isWakingUp) {
            GoToRunning();
          }
          motorController.RunForDuration(35);
          displayApp.PushMessage(Pinetime::Applications::Display::Messages::ShowPairingKey);
          break;

        default:
          break;
      }
    }

    if (isBleDiscoveryTimerRunning) {
      if (bleDiscoveryTimer == 0) {
        isBleDiscoveryTimerRunning = false;
        // Services discovery is deffered from 3 seconds to avoid the conflicts between the host communicating with the
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
    if (!nrf_gpio_pin_read(PinMap::Button)) {
      watchdog.Kick();
    }
  }
#pragma clang diagnostic pop
}

void SystemTask::UpdateMotion() {
  if (isGoingToSleep or isWakingUp) {
    return;
  }

  if (isSleeping && !(settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::RaiseWrist) ||
                      settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::Shake))) {
    return;
  }
  if (stepCounterMustBeReset) {
    motionSensor.ResetStepCounter();
    stepCounterMustBeReset = false;
  }

  auto motionValues = motionSensor.Process();

  motionController.IsSensorOk(motionSensor.IsOk());
  motionController.Update(motionValues.x, motionValues.y, motionValues.z, motionValues.steps);

  if (settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::RaiseWrist) &&
      motionController.Should_RaiseWake(isSleeping)) {
    GoToRunning();
  }
  if (settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::Shake) &&
      motionController.Should_ShakeWake(settingsController.GetShakeThreshold())) {
    GoToRunning();
  }
}

void SystemTask::HandleButtonAction(Controllers::ButtonActions action) {
  if (IsSleeping()) {
    return;
  }

  ReloadIdleTimer();

  using Actions = Controllers::ButtonActions;

  switch (action) {
    case Actions::Click:
      // If the first action after fast wakeup is a click, it should be ignored.
      if (!fastWakeUpDone && !isGoingToSleep) {
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
  if (isGoingToSleep or (not isSleeping) or isWakingUp) {
    return;
  }
  isWakingUp = true;
  PushMessage(Messages::GoToRunning);
}

void SystemTask::OnTouchEvent() {
  if (isGoingToSleep) {
    return;
  }
  if (!isSleeping) {
    PushMessage(Messages::OnTouchEvent);
  } else if (!isWakingUp) {
    if (settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::SingleTap) or
        settingsController.isWakeUpModeOn(Pinetime::Controllers::Settings::WakeUpMode::DoubleTap)) {
      PushMessage(Messages::TouchWakeUp);
    }
  }
}

void SystemTask::PushMessage(System::Messages msg) {
  if (msg == Messages::GoToSleep && !doNotGoToSleep) {
    isGoingToSleep = true;
  }

  if (in_isr()) {
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

void SystemTask::OnDim() {
  if (doNotGoToSleep) {
    return;
  }
  NRF_LOG_INFO("Dim timeout -> Dim screen")
  displayApp.PushMessage(Pinetime::Applications::Display::Messages::DimScreen);
  xTimerStart(idleTimer, 0);
  isDimmed = true;
}

void SystemTask::OnIdle() {
  if (doNotGoToSleep) {
    return;
  }
  NRF_LOG_INFO("Idle timeout -> Going to sleep")
  PushMessage(Messages::GoToSleep);
}

void SystemTask::ReloadIdleTimer() {
  if (isSleeping || isGoingToSleep) {
    return;
  }
  if (isDimmed) {
    displayApp.PushMessage(Pinetime::Applications::Display::Messages::RestoreBrightness);
    isDimmed = false;
  }
  xTimerReset(dimTimer, 0);
  xTimerStop(idleTimer, 0);
}
