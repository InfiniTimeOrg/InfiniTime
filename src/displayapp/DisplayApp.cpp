#include "displayapp/DisplayApp.h"
#include <libraries/log/nrf_log.h>
#include "displayapp/screens/HeartRate.h"
#include "displayapp/screens/Motion.h"
#include "displayapp/screens/Timer.h"
#include "displayapp/screens/Alarm.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/datetime/DateTimeController.h"
#include "components/ble/NotificationManager.h"
#include "components/motion/MotionController.h"
#include "components/motor/MotorController.h"
#include "displayapp/screens/ApplicationList.h"
#include "displayapp/screens/Clock.h"
#include "displayapp/screens/FirmwareUpdate.h"
#include "displayapp/screens/FirmwareValidation.h"
#include "displayapp/screens/InfiniPaint.h"
#include "displayapp/screens/Paddle.h"
#include "displayapp/screens/StopWatch.h"
#include "displayapp/screens/Metronome.h"
#include "displayapp/screens/Music.h"
#include "displayapp/screens/Navigation.h"
#include "displayapp/screens/Notifications.h"
#include "displayapp/screens/SystemInfo.h"
#include "displayapp/screens/Tile.h"
#include "displayapp/screens/Twos.h"
#include "displayapp/screens/FlashLight.h"
#include "displayapp/screens/BatteryInfo.h"
#include "displayapp/screens/Steps.h"
#include "displayapp/screens/PassKey.h"
#include "displayapp/screens/Error.h"

#include "drivers/Cst816s.h"
#include "drivers/St7789.h"
#include "drivers/Watchdog.h"
#include "systemtask/SystemTask.h"
#include "systemtask/Messages.h"

#include "displayapp/screens/settings/QuickSettings.h"
#include "displayapp/screens/settings/Settings.h"
#include "displayapp/screens/settings/SettingWatchFace.h"
#include "displayapp/screens/settings/SettingTimeFormat.h"
#include "displayapp/screens/settings/SettingWakeUp.h"
#include "displayapp/screens/settings/SettingDisplay.h"
#include "displayapp/screens/settings/SettingSteps.h"
#include "displayapp/screens/settings/SettingSetDate.h"
#include "displayapp/screens/settings/SettingSetTime.h"
#include "displayapp/screens/settings/SettingChimes.h"
#include "displayapp/screens/settings/SettingShakeThreshold.h"
#include "displayapp/screens/settings/SettingBluetooth.h"

#include "libs/lv_conf.h"

using namespace Pinetime::Applications;
using namespace Pinetime::Applications::Display;

namespace {
  static inline bool in_isr(void) {
    return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;
  }
}

DisplayApp::DisplayApp(Drivers::St7789& lcd,
                       Components::LittleVgl& lvgl,
                       Drivers::Cst816S& touchPanel,
                       Controllers::Battery& batteryController,
                       Controllers::Ble& bleController,
                       Controllers::DateTime& dateTimeController,
                       Drivers::WatchdogView& watchdog,
                       Pinetime::Controllers::NotificationManager& notificationManager,
                       Pinetime::Controllers::HeartRateController& heartRateController,
                       Controllers::Settings& settingsController,
                       Pinetime::Controllers::MotorController& motorController,
                       Pinetime::Controllers::MotionController& motionController,
                       Pinetime::Controllers::TimerController& timerController,
                       Pinetime::Controllers::AlarmController& alarmController,
                       Pinetime::Controllers::BrightnessController& brightnessController,
                       Pinetime::Controllers::TouchHandler& touchHandler)
  : lcd {lcd},
    lvgl {lvgl},
    touchPanel {touchPanel},
    batteryController {batteryController},
    bleController {bleController},
    dateTimeController {dateTimeController},
    watchdog {watchdog},
    notificationManager {notificationManager},
    heartRateController {heartRateController},
    settingsController {settingsController},
    motorController {motorController},
    motionController {motionController},
    timerController {timerController},
    alarmController {alarmController},
    brightnessController {brightnessController},
    touchHandler {touchHandler} {
}

void DisplayApp::Start(System::BootErrors error) {
  msgQueue = xQueueCreate(queueSize, itemSize);

  bootError = error;

  if (error == System::BootErrors::TouchController) {
    LoadApp(Apps::Error, DisplayApp::FullRefreshDirections::None);
  } else {
    LoadApp(Apps::Clock, DisplayApp::FullRefreshDirections::None);
  }

  if (pdPASS != xTaskCreate(DisplayApp::Process, "displayapp", 800, this, 0, &taskHandle)) {
    APP_ERROR_HANDLER(NRF_ERROR_NO_MEM);
  }
}

void DisplayApp::Process(void* instance) {
  auto* app = static_cast<DisplayApp*>(instance);
  NRF_LOG_INFO("displayapp task started!");
  app->InitHw();

  // Send a dummy notification to unlock the lvgl display driver for the first iteration
  xTaskNotifyGive(xTaskGetCurrentTaskHandle());

  while (true) {
    app->Refresh();
  }
}

void DisplayApp::InitHw() {
  brightnessController.Init();
  brightnessController.Set(settingsController.GetBrightness());
}

void DisplayApp::Refresh() {
  auto LoadPreviousScreen = [this]() {
    brightnessController.Set(settingsController.GetBrightness());
    LoadApp(returnToApp, returnDirection);
  };

  TickType_t queueTimeout;
  switch (state) {
    case States::Idle:
      queueTimeout = portMAX_DELAY;
      break;
    case States::Running:
      if (!currentScreen->IsRunning()) {
        LoadPreviousScreen();
      }
      queueTimeout = lv_task_handler();
      break;
    default:
      queueTimeout = portMAX_DELAY;
      break;
  }

  Messages msg;
  if (xQueueReceive(msgQueue, &msg, queueTimeout)) {
    switch (msg) {
      case Messages::DimScreen:
        brightnessController.Set(Controllers::BrightnessController::Levels::Low);
        break;
      case Messages::RestoreBrightness:
        brightnessController.Set(settingsController.GetBrightness());
        break;
      case Messages::GoToSleep:
        while (brightnessController.Level() != Controllers::BrightnessController::Levels::Off) {
          brightnessController.Lower();
          vTaskDelay(100);
        }
        PushMessageToSystemTask(Pinetime::System::Messages::OnDisplayTaskSleeping);
        state = States::Idle;
        break;
      case Messages::GoToRunning:
        brightnessController.Set(settingsController.GetBrightness());
        state = States::Running;
        break;
      case Messages::UpdateTimeOut:
        PushMessageToSystemTask(System::Messages::UpdateTimeOut);
        break;
      case Messages::UpdateBleConnection:
        //        clockScreen.SetBleConnectionState(bleController.IsConnected() ? Screens::Clock::BleConnectionStates::Connected :
        //        Screens::Clock::BleConnectionStates::NotConnected);
        break;
      case Messages::NewNotification:
        LoadApp(Apps::NotificationsPreview, DisplayApp::FullRefreshDirections::Down);
        break;
      case Messages::TimerDone:
        if (currentApp == Apps::Timer) {
          auto* timer = static_cast<Screens::Timer*>(currentScreen.get());
          timer->SetDone();
        } else {
          LoadApp(Apps::Timer, DisplayApp::FullRefreshDirections::Down);
        }
        break;
      case Messages::AlarmTriggered:
        if (currentApp == Apps::Alarm) {
          auto* alarm = static_cast<Screens::Alarm*>(currentScreen.get());
          alarm->SetAlerting();
        } else {
          LoadApp(Apps::Alarm, DisplayApp::FullRefreshDirections::None);
        }
        break;
      case Messages::ShowPairingKey:
        LoadApp(Apps::PassKey, DisplayApp::FullRefreshDirections::Up);
        break;
      case Messages::TouchEvent: {
        if (state != States::Running) {
          break;
        }
        auto gesture = touchHandler.GestureGet();
        if (gesture == TouchEvents::None) {
          break;
        }
        if (!currentScreen->OnTouchEvent(gesture)) {
          if (currentApp == Apps::Clock) {
            switch (gesture) {
              case TouchEvents::SwipeUp:
                LoadApp(Apps::Launcher, DisplayApp::FullRefreshDirections::Up);
                break;
              case TouchEvents::SwipeDown:
                LoadApp(Apps::Notifications, DisplayApp::FullRefreshDirections::Down);
                break;
              case TouchEvents::SwipeRight:
                LoadApp(Apps::QuickSettings, DisplayApp::FullRefreshDirections::RightAnim);
                break;
              case TouchEvents::DoubleTap:
                PushMessageToSystemTask(System::Messages::GoToSleep);
                break;
              default:
                break;
            }
          } else if (returnTouchEvent == gesture) {
            LoadPreviousScreen();
          }
        } else {
          touchHandler.CancelTap();
        }
      } break;
      case Messages::ButtonPushed:
        if (!currentScreen->OnButtonPushed()) {
          if (currentApp == Apps::Clock) {
            PushMessageToSystemTask(System::Messages::GoToSleep);
          } else {
            LoadPreviousScreen();
          }
        }
        break;
      case Messages::ButtonLongPressed:
        if (currentApp != Apps::Clock) {
          if (currentApp == Apps::Notifications) {
            LoadApp(Apps::Clock, DisplayApp::FullRefreshDirections::Up);
          } else if (currentApp == Apps::QuickSettings) {
            LoadApp(Apps::Clock, DisplayApp::FullRefreshDirections::LeftAnim);
          } else {
            LoadApp(Apps::Clock, DisplayApp::FullRefreshDirections::Down);
          }
        }
        break;
      case Messages::ButtonLongerPressed:
        // Create reboot app and open it instead
        LoadApp(Apps::SysInfo, DisplayApp::FullRefreshDirections::Up);
        break;
      case Messages::ButtonDoubleClicked:
        if (currentApp != Apps::Notifications && currentApp != Apps::NotificationsPreview) {
          LoadApp(Apps::Notifications, DisplayApp::FullRefreshDirections::Down);
        }
        break;

      case Messages::BleFirmwareUpdateStarted:
        LoadApp(Apps::FirmwareUpdate, DisplayApp::FullRefreshDirections::Down);
        break;
      case Messages::BleRadioEnableToggle:
        PushMessageToSystemTask(System::Messages::BleRadioEnableToggle);
        break;
      case Messages::UpdateDateTime:
        // Added to remove warning
        // What should happen here?
        break;
      case Messages::Clock:
        LoadApp(Apps::Clock, DisplayApp::FullRefreshDirections::None);
        break;
    }
  }

  if (touchHandler.IsTouching()) {
    currentScreen->OnTouchEvent(touchHandler.GetX(), touchHandler.GetY());
  }

  if (nextApp != Apps::None) {
    LoadApp(nextApp, nextDirection);
    nextApp = Apps::None;
  }
}

void DisplayApp::StartApp(Apps app, DisplayApp::FullRefreshDirections direction) {
  nextApp = app;
  nextDirection = direction;
}

void DisplayApp::ReturnApp(Apps app, DisplayApp::FullRefreshDirections direction, TouchEvents touchEvent) {
  returnToApp = app;
  returnDirection = direction;
  returnTouchEvent = touchEvent;
}

void DisplayApp::LoadApp(Apps app, DisplayApp::FullRefreshDirections direction) {
  touchHandler.CancelTap();
  currentScreen.reset(nullptr);
  SetFullRefresh(direction);

  // default return to launcher
  ReturnApp(Apps::Launcher, FullRefreshDirections::Down, TouchEvents::SwipeDown);

  switch (app) {
    case Apps::Launcher:
      currentScreen = std::make_unique<Screens::ApplicationList>(this, settingsController, batteryController, dateTimeController);
      ReturnApp(Apps::Clock, FullRefreshDirections::Down, TouchEvents::SwipeDown);
      break;
    case Apps::None:
    case Apps::Clock:
      currentScreen = std::make_unique<Screens::Clock>(this,
                                                       dateTimeController,
                                                       batteryController,
                                                       bleController,
                                                       notificationManager,
                                                       settingsController,
                                                       heartRateController,
                                                       motionController);
      break;

    case Apps::Error:
      currentScreen = std::make_unique<Screens::Error>(this, bootError);
      ReturnApp(Apps::Clock, FullRefreshDirections::Down, TouchEvents::None);
      break;

    case Apps::FirmwareValidation:
      currentScreen = std::make_unique<Screens::FirmwareValidation>(this, validator);
      ReturnApp(Apps::Settings, FullRefreshDirections::Down, TouchEvents::SwipeDown);
      break;
    case Apps::FirmwareUpdate:
      currentScreen = std::make_unique<Screens::FirmwareUpdate>(this, bleController);
      ReturnApp(Apps::Clock, FullRefreshDirections::Down, TouchEvents::None);
      break;

    case Apps::PassKey:
      currentScreen = std::make_unique<Screens::PassKey>(this, bleController.GetPairingKey());
      ReturnApp(Apps::Clock, FullRefreshDirections::Down, TouchEvents::SwipeDown);
      break;

    case Apps::Notifications:
      currentScreen = std::make_unique<Screens::Notifications>(this,
                                                               notificationManager,
                                                               systemTask->nimble().alertService(),
                                                               motorController,
                                                               *systemTask,
                                                               Screens::Notifications::Modes::Normal);
      ReturnApp(Apps::Clock, FullRefreshDirections::Up, TouchEvents::SwipeUp);
      break;
    case Apps::NotificationsPreview:
      currentScreen = std::make_unique<Screens::Notifications>(this,
                                                               notificationManager,
                                                               systemTask->nimble().alertService(),
                                                               motorController,
                                                               *systemTask,
                                                               Screens::Notifications::Modes::Preview);
      ReturnApp(Apps::Clock, FullRefreshDirections::Up, TouchEvents::SwipeUp);
      break;
    case Apps::Timer:
      currentScreen = std::make_unique<Screens::Timer>(this, timerController);
      break;
    case Apps::Alarm:
      currentScreen = std::make_unique<Screens::Alarm>(this, alarmController, settingsController, *systemTask);
      break;

    // Settings
    case Apps::QuickSettings:
      currentScreen = std::make_unique<Screens::QuickSettings>(this,
                                                               batteryController,
                                                               dateTimeController,
                                                               brightnessController,
                                                               motorController,
                                                               settingsController);
      ReturnApp(Apps::Clock, FullRefreshDirections::LeftAnim, TouchEvents::SwipeLeft);
      break;
    case Apps::Settings:
      currentScreen = std::make_unique<Screens::Settings>(this, settingsController);
      ReturnApp(Apps::QuickSettings, FullRefreshDirections::Down, TouchEvents::SwipeDown);
      break;
    case Apps::SettingWatchFace:
      currentScreen = std::make_unique<Screens::SettingWatchFace>(this, settingsController);
      ReturnApp(Apps::Settings, FullRefreshDirections::Down, TouchEvents::SwipeDown);
      break;
    case Apps::SettingTimeFormat:
      currentScreen = std::make_unique<Screens::SettingTimeFormat>(this, settingsController);
      ReturnApp(Apps::Settings, FullRefreshDirections::Down, TouchEvents::SwipeDown);
      break;
    case Apps::SettingWakeUp:
      currentScreen = std::make_unique<Screens::SettingWakeUp>(this, settingsController);
      ReturnApp(Apps::Settings, FullRefreshDirections::Down, TouchEvents::SwipeDown);
      break;
    case Apps::SettingDisplay:
      currentScreen = std::make_unique<Screens::SettingDisplay>(this, settingsController);
      ReturnApp(Apps::Settings, FullRefreshDirections::Down, TouchEvents::SwipeDown);
      break;
    case Apps::SettingSteps:
      currentScreen = std::make_unique<Screens::SettingSteps>(this, settingsController);
      ReturnApp(Apps::Settings, FullRefreshDirections::Down, TouchEvents::SwipeDown);
      break;
    case Apps::SettingSetDate:
      currentScreen = std::make_unique<Screens::SettingSetDate>(this, dateTimeController);
      ReturnApp(Apps::Settings, FullRefreshDirections::Down, TouchEvents::SwipeDown);
      break;
    case Apps::SettingSetTime:
      currentScreen = std::make_unique<Screens::SettingSetTime>(this, dateTimeController, settingsController);
      ReturnApp(Apps::Settings, FullRefreshDirections::Down, TouchEvents::SwipeDown);
      break;
    case Apps::SettingChimes:
      currentScreen = std::make_unique<Screens::SettingChimes>(this, settingsController);
      ReturnApp(Apps::Settings, FullRefreshDirections::Down, TouchEvents::SwipeDown);
      break;
    case Apps::SettingShakeThreshold:
      currentScreen = std::make_unique<Screens::SettingShakeThreshold>(this, settingsController, motionController, *systemTask);
      ReturnApp(Apps::Settings, FullRefreshDirections::Down, TouchEvents::SwipeDown);
      break;
    case Apps::SettingBluetooth:
      currentScreen = std::make_unique<Screens::SettingBluetooth>(this, settingsController);
      ReturnApp(Apps::Settings, FullRefreshDirections::Down, TouchEvents::SwipeDown);
      break;
    case Apps::BatteryInfo:
      currentScreen = std::make_unique<Screens::BatteryInfo>(this, batteryController);
      ReturnApp(Apps::Settings, FullRefreshDirections::Down, TouchEvents::SwipeDown);
      break;
    case Apps::SysInfo:
      currentScreen = std::make_unique<Screens::SystemInfo>(this,
                                                            dateTimeController,
                                                            batteryController,
                                                            brightnessController,
                                                            bleController,
                                                            watchdog,
                                                            motionController,
                                                            touchPanel);
      ReturnApp(Apps::Settings, FullRefreshDirections::Down, TouchEvents::SwipeDown);
      break;
    case Apps::FlashLight:
      currentScreen = std::make_unique<Screens::FlashLight>(this, *systemTask, brightnessController);
      ReturnApp(Apps::QuickSettings, FullRefreshDirections::Down, TouchEvents::SwipeDown);
      break;
    case Apps::StopWatch:
      currentScreen = std::make_unique<Screens::StopWatch>(this, *systemTask);
      break;
    case Apps::Twos:
      currentScreen = std::make_unique<Screens::Twos>(this);
      break;
    case Apps::Paint:
      currentScreen = std::make_unique<Screens::InfiniPaint>(this, lvgl, motorController);
      break;
    case Apps::Paddle:
      currentScreen = std::make_unique<Screens::Paddle>(this, lvgl);
      break;
    case Apps::Music:
      currentScreen = std::make_unique<Screens::Music>(this, systemTask->nimble().music());
      break;
    case Apps::Navigation:
      currentScreen = std::make_unique<Screens::Navigation>(this, systemTask->nimble().navigation());
      break;
    case Apps::HeartRate:
      currentScreen = std::make_unique<Screens::HeartRate>(this, heartRateController, *systemTask);
      break;
    case Apps::Metronome:
      currentScreen = std::make_unique<Screens::Metronome>(this, motorController, *systemTask);
      ReturnApp(Apps::Launcher, FullRefreshDirections::Down, TouchEvents::None);
      break;
    case Apps::Motion:
      currentScreen = std::make_unique<Screens::Motion>(this, motionController);
      break;
    case Apps::Steps:
      currentScreen = std::make_unique<Screens::Steps>(this, motionController, settingsController);
      break;
  }
  currentApp = app;
}

void DisplayApp::PushMessage(Messages msg) {
  if (in_isr()) {
    BaseType_t xHigherPriorityTaskWoken;
    xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(msgQueue, &msg, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken) {
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
  } else {
    xQueueSend(msgQueue, &msg, portMAX_DELAY);
  }
}

void DisplayApp::SetFullRefresh(DisplayApp::FullRefreshDirections direction) {
  switch (direction) {
    case DisplayApp::FullRefreshDirections::Down:
      lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::Down);
      break;
    case DisplayApp::FullRefreshDirections::Up:
      lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::Up);
      break;
    case DisplayApp::FullRefreshDirections::Left:
      lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::Left);
      break;
    case DisplayApp::FullRefreshDirections::Right:
      lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::Right);
      break;
    case DisplayApp::FullRefreshDirections::LeftAnim:
      lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::LeftAnim);
      break;
    case DisplayApp::FullRefreshDirections::RightAnim:
      lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::RightAnim);
      break;
    default:
      break;
  }
}

void DisplayApp::PushMessageToSystemTask(Pinetime::System::Messages message) {
  if (systemTask != nullptr) {
    systemTask->PushMessage(message);
  }
}

void DisplayApp::Register(Pinetime::System::SystemTask* systemTask) {
  this->systemTask = systemTask;
}
