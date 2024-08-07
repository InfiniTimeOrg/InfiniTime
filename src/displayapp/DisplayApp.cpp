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
#include "displayapp/screens/Dice.h"
#include "displayapp/screens/Weather.h"
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
#include "displayapp/screens/settings/SettingWeatherFormat.h"
#include "displayapp/screens/settings/SettingWakeUp.h"
#include "displayapp/screens/settings/SettingDisplay.h"
#include "displayapp/screens/settings/SettingSteps.h"
#include "displayapp/screens/settings/SettingSetDateTime.h"
#include "displayapp/screens/settings/SettingChimes.h"
#include "displayapp/screens/settings/SettingShakeThreshold.h"
#include "displayapp/screens/settings/SettingBluetooth.h"

#include "libs/lv_conf.h"
#include "UserApps.h"

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
                       Pinetime::Controllers::FS& filesystem,
                       Pinetime::Drivers::SpiNorFlash& spiNorFlash)
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
    spiNorFlash {spiNorFlash},
    lvgl {lcd, filesystem},
    timer(this, TimerCallback),
    controllers {batteryController,
                 bleController,
                 dateTimeController,
                 notificationManager,
                 heartRateController,
                 settingsController,
                 motorController,
                 motionController,
                 alarmController,
                 brightnessController,
                 nullptr,
                 filesystem,
                 timer,
                 nullptr,
                 this,
                 lvgl,
                 nullptr,
                 nullptr} {
}

void DisplayApp::Start(System::BootErrors error) {
  msgQueue = xQueueCreate(queueSize, itemSize);

  bootError = error;

  lvgl.Init();
  motorController.Init();

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

  while (true) {
    app->Refresh();
  }
}

void DisplayApp::InitHw() {
  brightnessController.Init();
  ApplyBrightness();
  lcd.Init();
}

TickType_t DisplayApp::CalculateSleepTime() {
  TickType_t ticksElapsed = xTaskGetTickCount() - alwaysOnStartTime;
  // Divide both the numerator and denominator by 8 to increase the number of ticks (frames) before the overflow tick is reached
  TickType_t elapsedTarget = ROUNDED_DIV((configTICK_RATE_HZ / 8) * alwaysOnTickCount * alwaysOnRefreshPeriod, 1000 / 8);
  // ROUNDED_DIV overflows when numerator + (denominator floordiv 2) > uint32 max
  // in this case around 9 hours
  constexpr TickType_t overflowTick = (UINT32_MAX - (1000 / 16)) / ((configTICK_RATE_HZ / 8) * alwaysOnRefreshPeriod);

  // Assumptions

  // Tick rate is multiple of 8
  // Needed for division trick above
  static_assert(configTICK_RATE_HZ % 8 == 0);

  // Local tick count must always wraparound before the system tick count does
  // As a static assert we can use 64 bit ints and therefore dodge overflows
  // Always on overflow time (ms) < system tick overflow time (ms)
  static_assert((uint64_t) overflowTick * (uint64_t) alwaysOnRefreshPeriod < (uint64_t) UINT32_MAX * 1000ULL / configTICK_RATE_HZ);

  if (alwaysOnTickCount == overflowTick) {
    alwaysOnTickCount = 0;
    alwaysOnStartTime = xTaskGetTickCount();
  }
  if (elapsedTarget > ticksElapsed) {
    return elapsedTarget - ticksElapsed;
  } else {
    return 0;
  }
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
      if (settingsController.GetAlwaysOnDisplay()) {
        if (!currentScreen->IsRunning()) {
          LoadPreviousScreen();
        }
        // Check we've slept long enough
        // Might not be true if the loop received an event
        // If not true, then wait that amount of time
        queueTimeout = CalculateSleepTime();
        if (queueTimeout == 0) {
          // Only advance the tick count when LVGL is done
          // Otherwise keep running the task handler while it still has things to draw
          // Note: under high graphics load, LVGL will always have more work to do
          if (lv_task_handler() > 0) {
            // Drop frames that we've missed if drawing/event handling took way longer than expected
            while (queueTimeout == 0) {
              alwaysOnTickCount += 1;
              queueTimeout = CalculateSleepTime();
            }
          };
        }
      } else {
        queueTimeout = portMAX_DELAY;
      }
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
      case Messages::GoToSleep:
        while (brightnessController.Level() != Controllers::BrightnessController::Levels::Low) {
          brightnessController.Lower();
          vTaskDelay(100);
        }
        // Don't actually turn off the display for AlwaysOn mode
        if (settingsController.GetAlwaysOnDisplay()) {
          brightnessController.Set(Controllers::BrightnessController::Levels::AlwaysOn);
          lcd.LowPowerOn();
          // Record idle entry time
          alwaysOnTickCount = 0;
          alwaysOnStartTime = xTaskGetTickCount();
        } else {
          brightnessController.Set(Controllers::BrightnessController::Levels::Off);
          lcd.Sleep();
        }
        PushMessageToSystemTask(Pinetime::System::Messages::OnDisplayTaskSleeping);
        state = States::Idle;
        break;
      case Messages::NotifyDeviceActivity:
        lv_disp_trig_activity(nullptr);
        break;
      case Messages::GoToRunning:
        if (settingsController.GetAlwaysOnDisplay()) {
          lcd.LowPowerOff();
        } else {
          lcd.Wakeup();
        }
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
        motorController.StartRinging();
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
    case Apps::Launcher: {
      std::array<Screens::Tile::Applications, UserAppTypes::Count> apps;
      int i = 0;
      for (const auto& userApp : userApps) {
        apps[i++] = Screens::Tile::Applications {userApp.icon, userApp.app, true};
      }
      currentScreen = std::make_unique<Screens::ApplicationList>(this,
                                                                 settingsController,
                                                                 batteryController,
                                                                 bleController,
                                                                 dateTimeController,
                                                                 filesystem,
                                                                 std::move(apps));
    } break;
    case Apps::Clock: {
      const auto* watchFace =
        std::find_if(userWatchFaces.begin(), userWatchFaces.end(), [this](const WatchFaceDescription& watchfaceDescription) {
          return watchfaceDescription.watchFace == settingsController.GetWatchFace();
        });
      if (watchFace != userWatchFaces.end())
        currentScreen.reset(watchFace->create(controllers));
      else {
        currentScreen.reset(userWatchFaces[0].create(controllers));
      }
      settingsController.SetAppMenu(0);
    } break;
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
    case Apps::SettingWatchFace: {
      std::array<Screens::SettingWatchFace::Item, UserWatchFaceTypes::Count> items;
      int i = 0;
      for (const auto& userWatchFace : userWatchFaces) {
        items[i++] =
          Screens::SettingWatchFace::Item {userWatchFace.name, userWatchFace.watchFace, userWatchFace.isAvailable(controllers.filesystem)};
      }
      currentScreen = std::make_unique<Screens::SettingWatchFace>(this, std::move(items), settingsController, filesystem);
    } break;
    case Apps::SettingTimeFormat:
      currentScreen = std::make_unique<Screens::SettingTimeFormat>(settingsController);
      break;
    case Apps::SettingWeatherFormat:
      currentScreen = std::make_unique<Screens::SettingWeatherFormat>(settingsController);
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
                                                            touchPanel,
                                                            spiNorFlash);
      break;
    case Apps::FlashLight:
      currentScreen = std::make_unique<Screens::FlashLight>(*systemTask, brightnessController);
      break;
    default: {
      const auto* d = std::find_if(userApps.begin(), userApps.end(), [app](const AppDescription& appDescription) {
        return appDescription.app == app;
      });
      if (d != userApps.end()) {
        currentScreen.reset(d->create(controllers));
      } else {
        currentScreen.reset(userWatchFaces[0].create(controllers));
      }
      break;
    }
  }
  currentApp = app;
}

void DisplayApp::PushMessage(Messages msg) {
  if (in_isr()) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(msgQueue, &msg, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  } else {
    TickType_t timeout = portMAX_DELAY;
    // Make xQueueSend() non-blocking if the message is a Notification message. We do this to avoid
    // deadlock between SystemTask and DisplayApp when their respective message queues are getting full
    // when a lot of notifications are received on a very short time span.
    if (msg == Messages::NewNotification) {
      timeout = static_cast<TickType_t>(0);
    }

    xQueueSend(msgQueue, &msg, timeout);
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
  this->controllers.systemTask = systemTask;
}

void DisplayApp::Register(Pinetime::Controllers::SimpleWeatherService* weatherService) {
  this->controllers.weatherController = weatherService;
}

void DisplayApp::Register(Pinetime::Controllers::MusicService* musicService) {
  this->controllers.musicService = musicService;
}

void DisplayApp::Register(Pinetime::Controllers::NavigationService* NavigationService) {
  this->controllers.navigationService = NavigationService;
}

void DisplayApp::ApplyBrightness() {
  auto brightness = settingsController.GetBrightness();
  if (brightness != Controllers::BrightnessController::Levels::Low && brightness != Controllers::BrightnessController::Levels::Medium &&
      brightness != Controllers::BrightnessController::Levels::High) {
    brightness = Controllers::BrightnessController::Levels::High;
  }
  brightnessController.Set(brightness);
}
