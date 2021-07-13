#include "DisplayApp.h"
#include <libraries/log/nrf_log.h>
#include <displayapp/screens/HeartRate.h>
#include <displayapp/screens/Motion.h>
#include <displayapp/screens/Timer.h>
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/datetime/DateTimeController.h"
#include "components/ble/NotificationManager.h"
#include "components/motion/MotionController.h"
#include "components/motor/MotorController.h"
#include "displayapp/screens/ApplicationList.h"
#include "displayapp/screens/Brightness.h"
#include "displayapp/screens/Clock.h"
#include "displayapp/screens/FirmwareUpdate.h"
#include "displayapp/screens/FirmwareValidation.h"
#include "displayapp/screens/InfiniPaint.h"
#include "displayapp/screens/Paddle.h"
#include "displayapp/screens/StopWatch.h"
#include "displayapp/screens/Meter.h"
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
                       Pinetime::Controllers::TimerController& timerController)
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
    timerController {timerController} {
}

void DisplayApp::Start() {
  msgQueue = xQueueCreate(queueSize, itemSize);

  // Start clock when smartwatch boots
  LoadApp(Apps::Clock, DisplayApp::FullRefreshDirections::None);

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

uint32_t acc = 0;
uint32_t count = 0;
bool toggle = true;
void DisplayApp::Refresh() {
  TickType_t queueTimeout;
  TickType_t delta;
  switch (state) {
    case States::Idle:
      IdleState();
      queueTimeout = portMAX_DELAY;
      break;
    case States::Running:
      RunningState();
      delta = xTaskGetTickCount() - lastWakeTime;
      if (delta > LV_DISP_DEF_REFR_PERIOD) {
        delta = LV_DISP_DEF_REFR_PERIOD;
      }
      queueTimeout = LV_DISP_DEF_REFR_PERIOD - delta;
      break;
    default:
      queueTimeout = portMAX_DELAY;
      break;
  }

  Messages msg;
  bool messageReceived = xQueueReceive(msgQueue, &msg, queueTimeout);
  lastWakeTime = xTaskGetTickCount();
  if (messageReceived) {
    switch (msg) {
      case Messages::GoToSleep:
        brightnessController.Backup();
        while (brightnessController.Level() != Controllers::BrightnessController::Levels::Off) {
          brightnessController.Lower();
          vTaskDelay(100);
        }
        lcd.DisplayOff();
        PushMessageToSystemTask(Pinetime::System::Messages::OnDisplayTaskSleeping);
        state = States::Idle;
        break;
      case Messages::GoToRunning:
        lcd.DisplayOn();
        brightnessController.Restore();
        state = States::Running;
        break;
      case Messages::UpdateTimeOut:
        PushMessageToSystemTask(System::Messages::UpdateTimeOut);
        break;
      case Messages::UpdateBleConnection:
        //        clockScreen.SetBleConnectionState(bleController.IsConnected() ? Screens::Clock::BleConnectionStates::Connected :
        //        Screens::Clock::BleConnectionStates::NotConnected);
        break;
      case Messages::UpdateBatteryLevel:
        batteryController.Update();
        break;
      case Messages::NewNotification:
        LoadApp(Apps::NotificationsPreview, DisplayApp::FullRefreshDirections::Down);
        break;
      case Messages::TimerDone:
        if (currentApp == Apps::Timer) {
          auto *timer = static_cast<Screens::Timer*>(currentScreen.get());
          timer->setDone();
        } else {
          LoadApp(Apps::Timer, DisplayApp::FullRefreshDirections::Down);
        }
        break;
      case Messages::TouchEvent: {
        if (state != States::Running)
          break;
        auto gesture = OnTouchEvent();
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
            LoadApp(returnToApp, returnDirection);
          }
        }
      } break;
      case Messages::ButtonPushed:
        if (currentApp == Apps::Clock) {
          PushMessageToSystemTask(System::Messages::GoToSleep);
        } else {
          if (!currentScreen->OnButtonPushed()) {
            LoadApp(returnToApp, returnDirection);
          }
        }
        break;

      case Messages::BleFirmwareUpdateStarted:
        LoadApp(Apps::FirmwareUpdate, DisplayApp::FullRefreshDirections::Down);
        break;
      case Messages::UpdateDateTime:
        // Added to remove warning
        // What should happen here?
        break;
    }
  }

  if(nextApp != Apps::None) {
    LoadApp(nextApp, nextDirection);
    nextApp = Apps::None;
  }

  if (state != States::Idle && touchMode == TouchModes::Polling) {
    auto info = touchPanel.GetTouchInfo();
    if (info.action == 2) { // 2 = contact
      if (!currentScreen->OnTouchEvent(info.x, info.y)) {
        lvgl.SetNewTapEvent(info.x, info.y);
      }
    }
  }
}

void DisplayApp::RunningState() {
  if (!currentScreen->Refresh()) {
    LoadApp(returnToApp, returnDirection);
  }
  lv_task_handler();
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

    case Apps::FirmwareValidation:
      currentScreen = std::make_unique<Screens::FirmwareValidation>(this, validator);
      ReturnApp(Apps::Settings, FullRefreshDirections::Down, TouchEvents::SwipeDown);
      break;
    case Apps::FirmwareUpdate:
      currentScreen = std::make_unique<Screens::FirmwareUpdate>(this, bleController);
      break;

    case Apps::Notifications:
      currentScreen = std::make_unique<Screens::Notifications>(
        this, notificationManager, systemTask->nimble().alertService(), Screens::Notifications::Modes::Normal);
      ReturnApp(Apps::Clock, FullRefreshDirections::Up, TouchEvents::SwipeUp);
      break;
    case Apps::NotificationsPreview:
      currentScreen = std::make_unique<Screens::Notifications>(
        this, notificationManager, systemTask->nimble().alertService(), Screens::Notifications::Modes::Preview);
      ReturnApp(Apps::Clock, FullRefreshDirections::Up, TouchEvents::SwipeUp);
      break;
    case Apps::Timer:
      currentScreen = std::make_unique<Screens::Timer>(this, timerController);
      break;

    // Settings
    case Apps::QuickSettings:
      currentScreen = std::make_unique<Screens::QuickSettings>(
        this, batteryController, dateTimeController, brightnessController, motorController, settingsController);
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
    case Apps::BatteryInfo:
      currentScreen = std::make_unique<Screens::BatteryInfo>(this, batteryController);
      ReturnApp(Apps::Settings, FullRefreshDirections::Down, TouchEvents::SwipeDown);
      break;
    case Apps::SysInfo:
      currentScreen =
        std::make_unique<Screens::SystemInfo>(this, dateTimeController, batteryController, brightnessController, bleController, watchdog, motionController);
      ReturnApp(Apps::Settings, FullRefreshDirections::Down, TouchEvents::SwipeDown);
      break;
    case Apps::FlashLight:
      currentScreen = std::make_unique<Screens::FlashLight>(this, *systemTask, brightnessController);
      ReturnApp(Apps::Clock, FullRefreshDirections::Down, TouchEvents::None);
      break;
    case Apps::StopWatch:
      currentScreen = std::make_unique<Screens::StopWatch>(this, *systemTask);
      break;
    case Apps::Twos:
      currentScreen = std::make_unique<Screens::Twos>(this);
      break;
    case Apps::Paint:
      currentScreen = std::make_unique<Screens::InfiniPaint>(this, lvgl);
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

void DisplayApp::IdleState() {
}

void DisplayApp::PushMessage(Messages msg) {
  if(in_isr()) {
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

TouchEvents DisplayApp::OnTouchEvent() {
  auto info = touchPanel.GetTouchInfo();
  if (info.isTouch) {
    switch (info.gesture) {
      case Pinetime::Drivers::Cst816S::Gestures::SingleTap:
        if (touchMode == TouchModes::Gestures) {
          lvgl.SetNewTapEvent(info.x, info.y);
        }
        return TouchEvents::Tap;
      case Pinetime::Drivers::Cst816S::Gestures::LongPress:
        return TouchEvents::LongTap;
      case Pinetime::Drivers::Cst816S::Gestures::DoubleTap:
        return TouchEvents::DoubleTap;
      case Pinetime::Drivers::Cst816S::Gestures::SlideRight:
        return TouchEvents::SwipeRight;
      case Pinetime::Drivers::Cst816S::Gestures::SlideLeft:
        return TouchEvents::SwipeLeft;
      case Pinetime::Drivers::Cst816S::Gestures::SlideDown:
        return TouchEvents::SwipeDown;
      case Pinetime::Drivers::Cst816S::Gestures::SlideUp:
        return TouchEvents::SwipeUp;
      case Pinetime::Drivers::Cst816S::Gestures::None:
      default:
        return TouchEvents::None;
    }
  }
  return TouchEvents::None;
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

void DisplayApp::SetTouchMode(DisplayApp::TouchModes mode) {
  touchMode = mode;
}

void DisplayApp::PushMessageToSystemTask(Pinetime::System::Messages message) {
  if(systemTask != nullptr)
    systemTask->PushMessage(message);
}

void DisplayApp::Register(Pinetime::System::SystemTask* systemTask) {
  this->systemTask = systemTask;
}
