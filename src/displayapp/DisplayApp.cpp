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
#include "displayapp/screens/Weather.h"

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
#include "displayapp/screens/settings/SettingSetDateTime.h"
#include "displayapp/screens/settings/SettingChimes.h"
#include "displayapp/screens/settings/SettingShakeThreshold.h"
#include "displayapp/screens/settings/SettingBluetooth.h"

#include "libs/lv_conf.h"

using namespace Pinetime::Applications;
using namespace Pinetime::Applications::Display;

namespace {
  inline bool in_isr() {
    return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;
  }

  void TimerCallback(TimerHandle_t xTimer) {
    auto* dispApp = static_cast<DisplayApp*>(pvTimerGetTimerID(xTimer));
    dispApp->PushMessage(Display::Messages::TimerDone);
  }
}

DisplayApp::DisplayApp(Drivers::St7789& lcd,
                       const Drivers::Cst816S& touchPanel,
                       const Controllers::Battery& batteryController,
                       const Controllers::Ble& bleController,
                       Controllers::DateTime& dateTimeController,
                       const Drivers::Watchdog& watchdog,
                       Pinetime::Controllers::NotificationManager& notificationManager,
                       Pinetime::Controllers::HeartRateController& heartRateController,
                       Controllers::Settings& settingsController,
                       Pinetime::Controllers::MotorController& motorController,
                       Pinetime::Controllers::MotionController& motionController,
                       Pinetime::Controllers::AlarmController& alarmController,
                       Pinetime::Controllers::BrightnessController& brightnessController,
                       Pinetime::Controllers::TouchHandler& touchHandler,
                       Pinetime::Controllers::FS& filesystem)
  : lcd {lcd},
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
    alarmController {alarmController},
    brightnessController {brightnessController},
    touchHandler {touchHandler},
    filesystem {filesystem},
    lvgl {lcd, filesystem},
    timer(this, TimerCallback),
    popupMessage {"Touch input\nis ignored,\npush button\nto unlock."} {
}

void DisplayApp::Start(System::BootErrors error) {
  msgQueue = xQueueCreate(queueSize, itemSize);

  bootError = error;

  lvgl.Init();

  if (error == System::BootErrors::TouchController) {
    LoadNewScreen(Apps::Error, DisplayApp::FullRefreshDirections::None);
  } else {
    LoadNewScreen(Apps::Clock, DisplayApp::FullRefreshDirections::None);
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
  ApplyBrightness();
  motorController.Init();
  lcd.Init();
}

void DisplayApp::Refresh() {
  auto LoadPreviousScreen = [this]() {
    FullRefreshDirections returnDirection;
    switch (appStackDirections.Pop()) {
      case FullRefreshDirections::Up:
        returnDirection = FullRefreshDirections::Down;
        break;
      case FullRefreshDirections::Down:
        returnDirection = FullRefreshDirections::Up;
        break;
      case FullRefreshDirections::LeftAnim:
        returnDirection = FullRefreshDirections::RightAnim;
        break;
      case FullRefreshDirections::RightAnim:
        returnDirection = FullRefreshDirections::LeftAnim;
        break;
      default:
        returnDirection = FullRefreshDirections::None;
        break;
    }
    LoadScreen(returnAppStack.Pop(), returnDirection);
  };

  auto DimScreen = [this]() {
    if (brightnessController.Level() != Controllers::BrightnessController::Levels::Off) {
      isDimmed = true;
      brightnessController.Set(Controllers::BrightnessController::Levels::Low);
    }
  };

  auto RestoreBrightness = [this]() {
    if (brightnessController.Level() != Controllers::BrightnessController::Levels::Off) {
      isDimmed = false;
      lv_disp_trig_activity(nullptr);
      ApplyBrightness();
    }
  };

  auto IsPastDimTime = [this]() -> bool {
    return lv_disp_get_inactive_time(nullptr) >= pdMS_TO_TICKS(settingsController.GetScreenTimeOut() - 2000);
  };

  auto IsPastSleepTime = [this]() -> bool {
    return lv_disp_get_inactive_time(nullptr) >= pdMS_TO_TICKS(settingsController.GetScreenTimeOut());
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

      if (!systemTask->IsSleepDisabled() && IsPastDimTime()) {
        if (!isDimmed) {
          DimScreen();
        }
        if (IsPastSleepTime()) {
          systemTask->PushMessage(System::Messages::GoToSleep);
          state = States::Idle;
        }
      } else if (isDimmed) {
        RestoreBrightness();
      }
      break;
    default:
      queueTimeout = portMAX_DELAY;
      break;
  }

  Messages msg;
  if (xQueueReceive(msgQueue, &msg, queueTimeout) == pdTRUE) {
    switch (msg) {
      case Messages::DimScreen:
        DimScreen();
        break;
      case Messages::RestoreBrightness:
        RestoreBrightness();
        break;
      case Messages::GoToSleep:
        while (brightnessController.Level() != Controllers::BrightnessController::Levels::Off) {
          brightnessController.Lower();
          vTaskDelay(100);
        }
        lcd.Sleep();
        PushMessageToSystemTask(Pinetime::System::Messages::OnDisplayTaskSleeping);
        state = States::Idle;
        break;
      case Messages::GoToRunning:
        lcd.Wakeup();
        lv_disp_trig_activity(nullptr);
        ApplyBrightness();
        state = States::Running;
        break;
      case Messages::UpdateBleConnection:
        //        clockScreen.SetBleConnectionState(bleController.IsConnected() ? Screens::Clock::BleConnectionStates::Connected :
        //        Screens::Clock::BleConnectionStates::NotConnected);
        break;
      case Messages::NewNotification:
        LoadNewScreen(Apps::NotificationsPreview, DisplayApp::FullRefreshDirections::Down);
        break;
      case Messages::TimerDone:
        if (state != States::Running) {
          PushMessageToSystemTask(System::Messages::GoToRunning);
        }
        if (currentApp == Apps::Timer) {
          lv_disp_trig_activity(nullptr);
          auto* timer = static_cast<Screens::Timer*>(currentScreen.get());
          timer->Reset();
        } else {
          LoadNewScreen(Apps::Timer, DisplayApp::FullRefreshDirections::Up);
        }
        motorController.RunForDuration(35);
        break;
      case Messages::AlarmTriggered:
        if (currentApp == Apps::Alarm) {
          auto* alarm = static_cast<Screens::Alarm*>(currentScreen.get());
          alarm->SetAlerting();
        } else {
          LoadNewScreen(Apps::Alarm, DisplayApp::FullRefreshDirections::None);
        }
        break;
      case Messages::ShowPairingKey:
        LoadNewScreen(Apps::PassKey, DisplayApp::FullRefreshDirections::Up);
        motorController.RunForDuration(35);
        break;
      case Messages::TouchEvent: {
        if (state != States::Running) {
          break;
        }
        lvgl.SetNewTouchPoint(touchHandler.GetX(), touchHandler.GetY(), touchHandler.IsTouching());
        auto gesture = touchHandler.GestureGet();
        if (gesture == TouchEvents::None) {
          break;
        }
        auto LoadDirToReturnSwipe = [](DisplayApp::FullRefreshDirections refreshDirection) {
          switch (refreshDirection) {
            default:
            case DisplayApp::FullRefreshDirections::Up:
              return TouchEvents::SwipeDown;
            case DisplayApp::FullRefreshDirections::Down:
              return TouchEvents::SwipeUp;
            case DisplayApp::FullRefreshDirections::LeftAnim:
              return TouchEvents::SwipeRight;
            case DisplayApp::FullRefreshDirections::RightAnim:
              return TouchEvents::SwipeLeft;
          }
        };
        if (!currentScreen->OnTouchEvent(gesture)) {
          if (currentApp == Apps::Clock) {
            switch (gesture) {
              case TouchEvents::SwipeUp:
                LoadNewScreen(Apps::Launcher, DisplayApp::FullRefreshDirections::Up);
                break;
              case TouchEvents::SwipeDown:
                LoadNewScreen(Apps::Notifications, DisplayApp::FullRefreshDirections::Down);
                break;
              case TouchEvents::SwipeRight:
                LoadNewScreen(Apps::QuickSettings, DisplayApp::FullRefreshDirections::RightAnim);
                break;
              case TouchEvents::DoubleTap:
                PushMessageToSystemTask(System::Messages::GoToSleep);
                break;
              default:
                break;
            }
          } else if (gesture == LoadDirToReturnSwipe(appStackDirections.Top())) {
            LoadPreviousScreen();
          }
        } else {
          lvgl.CancelTap();
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
            LoadNewScreen(Apps::Clock, DisplayApp::FullRefreshDirections::Up);
          } else if (currentApp == Apps::QuickSettings) {
            LoadNewScreen(Apps::Clock, DisplayApp::FullRefreshDirections::LeftAnim);
          } else {
            LoadNewScreen(Apps::Clock, DisplayApp::FullRefreshDirections::Down);
          }
          appStackDirections.Reset();
          returnAppStack.Reset();
        }
        break;
      case Messages::ButtonLongerPressed:
        // Create reboot app and open it instead
        LoadNewScreen(Apps::SysInfo, DisplayApp::FullRefreshDirections::Up);
        break;
      case Messages::ButtonDoubleClicked:
        if (currentApp != Apps::Notifications && currentApp != Apps::NotificationsPreview) {
          LoadNewScreen(Apps::Notifications, DisplayApp::FullRefreshDirections::Down);
        }
        break;

      case Messages::BleFirmwareUpdateStarted:
        LoadNewScreen(Apps::FirmwareUpdate, DisplayApp::FullRefreshDirections::Down);
        break;
      case Messages::BleRadioEnableToggle:
        PushMessageToSystemTask(System::Messages::BleRadioEnableToggle);
        break;
      case Messages::UpdateDateTime:
        // Added to remove warning
        // What should happen here?
        break;
      case Messages::Chime:
        LoadNewScreen(Apps::Clock, DisplayApp::FullRefreshDirections::None);
        motorController.RunForDuration(35);
        break;
      case Messages::OnChargingEvent:
        RestoreBrightness();
        motorController.RunForDuration(15);
        break;
      case Messages::ShowIgnoreTouchPopup:
        popupMessage.SetHidden(false);
        break;
      case Messages::HideIgnoreTouchPopup:
        popupMessage.SetHidden(true);
        break;
    }
  }

  if (touchHandler.IsTouching()) {
    currentScreen->OnTouchEvent(touchHandler.GetX(), touchHandler.GetY());
  }

  if (nextApp != Apps::None) {
    LoadNewScreen(nextApp, nextDirection);
    nextApp = Apps::None;
  }
}

void DisplayApp::StartApp(Apps app, DisplayApp::FullRefreshDirections direction) {
  nextApp = app;
  nextDirection = direction;
}

void DisplayApp::LoadNewScreen(Apps app, DisplayApp::FullRefreshDirections direction) {
  // Don't add the same screen to the stack back to back.
  // This is mainly to fix an issue with receiving two notifications at the same time
  // and shouldn't happen otherwise.
  if (app != currentApp) {
    returnAppStack.Push(currentApp);
    appStackDirections.Push(direction);
  }
  LoadScreen(app, direction);
}

void DisplayApp::LoadScreen(Apps app, DisplayApp::FullRefreshDirections direction) {
  lvgl.CancelTap();
  lv_disp_trig_activity(nullptr);
  motorController.StopRinging();

  currentScreen.reset(nullptr);
  SetFullRefresh(direction);

  switch (app) {
    case Apps::Launcher:
      currentScreen =
        std::make_unique<Screens::ApplicationList>(this, settingsController, batteryController, bleController, dateTimeController, filesystem);
      break;
    case Apps::Motion:
      // currentScreen = std::make_unique<Screens::Motion>(motionController);
      // break;
    case Apps::None:
    case Apps::Clock:
      currentScreen = std::make_unique<Screens::Clock>(dateTimeController,
                                                       batteryController,
                                                       bleController,
                                                       notificationManager,
                                                       settingsController,
                                                       heartRateController,
                                                       motionController,
                                                       systemTask->nimble().weather(),
                                                       filesystem);
      break;

    case Apps::Error:
      currentScreen = std::make_unique<Screens::Error>(bootError);
      break;

    case Apps::FirmwareValidation:
      currentScreen = std::make_unique<Screens::FirmwareValidation>(validator);
      break;
    case Apps::FirmwareUpdate:
      currentScreen = std::make_unique<Screens::FirmwareUpdate>(bleController);
      break;

    case Apps::PassKey:
      currentScreen = std::make_unique<Screens::PassKey>(bleController.GetPairingKey());
      break;

    case Apps::Notifications:
      currentScreen = std::make_unique<Screens::Notifications>(this,
                                                               notificationManager,
                                                               systemTask->nimble().alertService(),
                                                               motorController,
                                                               *systemTask,
                                                               Screens::Notifications::Modes::Normal);
      break;
    case Apps::NotificationsPreview:
      currentScreen = std::make_unique<Screens::Notifications>(this,
                                                               notificationManager,
                                                               systemTask->nimble().alertService(),
                                                               motorController,
                                                               *systemTask,
                                                               Screens::Notifications::Modes::Preview);
      break;
    case Apps::Timer:
      currentScreen = std::make_unique<Screens::Timer>(timer);
      break;
    case Apps::Alarm:
      currentScreen = std::make_unique<Screens::Alarm>(alarmController, settingsController.GetClockType(), *systemTask, motorController);
      break;

    // Settings
    case Apps::QuickSettings:
      currentScreen = std::make_unique<Screens::QuickSettings>(this,
                                                               batteryController,
                                                               dateTimeController,
                                                               brightnessController,
                                                               motorController,
                                                               settingsController,
                                                               bleController);
      break;
    case Apps::Settings:
      currentScreen = std::make_unique<Screens::Settings>(this, settingsController);
      break;
    case Apps::SettingWatchFace:
      currentScreen = std::make_unique<Screens::SettingWatchFace>(this, settingsController, filesystem);
      break;
    case Apps::SettingTimeFormat:
      currentScreen = std::make_unique<Screens::SettingTimeFormat>(settingsController);
      break;
    case Apps::SettingWakeUp:
      currentScreen = std::make_unique<Screens::SettingWakeUp>(settingsController);
      break;
    case Apps::SettingDisplay:
      currentScreen = std::make_unique<Screens::SettingDisplay>(this, settingsController);
      break;
    case Apps::SettingSteps:
      currentScreen = std::make_unique<Screens::SettingSteps>(settingsController);
      break;
    case Apps::SettingSetDateTime:
      currentScreen = std::make_unique<Screens::SettingSetDateTime>(this, dateTimeController, settingsController);
      break;
    case Apps::SettingChimes:
      currentScreen = std::make_unique<Screens::SettingChimes>(settingsController);
      break;
    case Apps::SettingShakeThreshold:
      currentScreen = std::make_unique<Screens::SettingShakeThreshold>(settingsController, motionController, *systemTask);
      break;
    case Apps::SettingBluetooth:
      currentScreen = std::make_unique<Screens::SettingBluetooth>(this, settingsController);
      break;
    case Apps::BatteryInfo:
      currentScreen = std::make_unique<Screens::BatteryInfo>(batteryController);
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
      break;
    case Apps::FlashLight:
      currentScreen = std::make_unique<Screens::FlashLight>(*systemTask, brightnessController);
      break;
    case Apps::StopWatch:
      currentScreen = std::make_unique<Screens::StopWatch>(*systemTask);
      break;
    case Apps::Twos:
      currentScreen = std::make_unique<Screens::Twos>();
      break;
    case Apps::Paint:
      currentScreen = std::make_unique<Screens::InfiniPaint>(lvgl, motorController);
      break;
    case Apps::Paddle:
      currentScreen = std::make_unique<Screens::Paddle>(lvgl);
      break;
    case Apps::Music:
      currentScreen = std::make_unique<Screens::Music>(systemTask->nimble().music());
      break;
    case Apps::Navigation:
      currentScreen = std::make_unique<Screens::Navigation>(systemTask->nimble().navigation());
      break;
    case Apps::HeartRate:
      currentScreen = std::make_unique<Screens::HeartRate>(heartRateController, *systemTask);
      break;
    case Apps::Metronome:
      currentScreen = std::make_unique<Screens::Metronome>(motorController, *systemTask);
      break;
    /* Weather debug app
    case Apps::Weather:
      currentScreen = std::make_unique<Screens::Weather>(this, systemTask->nimble().weather());
      break;
    */
    case Apps::Steps:
      currentScreen = std::make_unique<Screens::Steps>(motionController, settingsController);
      break;
  }
  currentApp = app;
}

void DisplayApp::PushMessage(Messages msg) {
  if (in_isr()) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(msgQueue, &msg, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE) {
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

void DisplayApp::ApplyBrightness() {
  auto brightness = settingsController.GetBrightness();
  if (brightness != Controllers::BrightnessController::Levels::Low && brightness != Controllers::BrightnessController::Levels::Medium &&
      brightness != Controllers::BrightnessController::Levels::High) {
    brightness = Controllers::BrightnessController::Levels::High;
  }
  brightnessController.Set(brightness);
}
