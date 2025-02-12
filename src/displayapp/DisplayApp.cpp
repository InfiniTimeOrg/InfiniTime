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
#include "displayapp/screens/settings/SettingHeartRate.h"
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
  // Calculates how many system ticks DisplayApp should sleep before rendering the next AOD frame
  // Next frame time is frame count * refresh period (ms) * tick rate

  auto RoundedDiv = [](uint32_t a, uint32_t b) {
    return ((a + (b / 2)) / b);
  };
  // RoundedDiv overflows when numerator + (denominator floordiv 2) > uint32 max
  // in this case around 9 hours (=overflow frame count / always on refresh period)
  constexpr TickType_t overflowFrameCount = (UINT32_MAX - (1000 / 16)) / ((configTICK_RATE_HZ / 8) * alwaysOnRefreshPeriod);

  TickType_t ticksElapsed = xTaskGetTickCount() - alwaysOnStartTime;
  // Divide both the numerator and denominator by 8 (=GCD(1000,1024))
  // to increase the number of ticks (frames) before the overflow tick is reached
  TickType_t targetRenderTick = RoundedDiv((configTICK_RATE_HZ / 8) * alwaysOnFrameCount * alwaysOnRefreshPeriod, 1000 / 8);

  // Assumptions

  // Tick rate is multiple of 8
  // Needed for division trick above
  static_assert(configTICK_RATE_HZ % 8 == 0);

  // Frame count must always wraparound more often than the system tick count does
  // Always on overflow time (ms) < system tick overflow time (ms)
  // Using 64bit ints here to avoid overflow
  static_assert((uint64_t) overflowFrameCount * (uint64_t) alwaysOnRefreshPeriod < (uint64_t) UINT32_MAX * 1000ULL / configTICK_RATE_HZ);

  if (alwaysOnFrameCount == overflowFrameCount) {
    alwaysOnFrameCount = 0;
    alwaysOnStartTime = xTaskGetTickCount();
  }
  if (targetRenderTick > ticksElapsed) {
    return targetRenderTick - ticksElapsed;
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
    case States::AOD:
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
            alwaysOnFrameCount += 1;
            queueTimeout = CalculateSleepTime();
          }
        }
      }
      break;
    case States::Running:
      if (!currentScreen->IsRunning()) {
        LoadPreviousScreen();
      }
      queueTimeout = lv_task_handler();

      if (!systemTask->IsSleepDisabled() && IsPastDimTime()) {
        if (!isDimmed) {
          isDimmed = true;
          brightnessController.Set(Controllers::BrightnessController::Levels::Low);
        }
        if (IsPastSleepTime() && uxQueueMessagesWaiting(msgQueue) == 0) {
          PushMessageToSystemTask(System::Messages::GoToSleep);
          // Can't set state to Idle here, something may send
          // DisableSleeping before this GoToSleep arrives
          // Instead we check we have no messages queued before sending GoToSleep
          // This works as the SystemTask is higher priority than DisplayApp
          // As soon as we send GoToSleep, SystemTask pre-empts DisplayApp
          // Whenever DisplayApp is running again, it is guaranteed that
          // SystemTask has handled the message
          // If it responded, we will have a GoToSleep waiting in the queue
          // By checking that there are no messages in the queue, we avoid
          // resending GoToSleep when we already have a response
          // SystemTask is resilient to duplicate messages, this is an
          // optimisation to reduce pressure on the message queues
        }
      } else if (isDimmed) {
        isDimmed = false;
        ApplyBrightness();
      }
      break;
    default:
      queueTimeout = portMAX_DELAY;
      break;
  }

  Messages msg;
  if (xQueueReceive(msgQueue, &msg, queueTimeout) == pdTRUE) {
    switch (msg) {
      case Messages::GoToSleep:
      case Messages::GoToAOD:
        // Checking if SystemTask is sleeping is purely an optimisation.
        // If it's no longer sleeping since it sent GoToSleep, it has
        // cancelled the sleep and transitioned directly from
        // GoingToSleep->Running, so we are about to receive GoToRunning
        // and can ignore this message. If it wasn't ignored, DisplayApp
        // would go to sleep and then immediately re-wake
        if (state != States::Running || !systemTask->IsSleeping()) {
          break;
        }
        while (brightnessController.Level() != Controllers::BrightnessController::Levels::Low) {
          brightnessController.Lower();
          vTaskDelay(100);
        }
        // Turn brightness down (or set to AlwaysOn mode)
        if (msg == Messages::GoToAOD) {
          brightnessController.Set(Controllers::BrightnessController::Levels::AlwaysOn);
        } else {
          brightnessController.Set(Controllers::BrightnessController::Levels::Off);
        }
        // Since the active screen is not really an app, go back to Clock.
        if (currentApp == Apps::Launcher || currentApp == Apps::Notifications || currentApp == Apps::QuickSettings ||
            currentApp == Apps::Settings) {
          LoadScreen(Apps::Clock, DisplayApp::FullRefreshDirections::None);
          // Wait for the clock app to load before moving on.
          while (!lv_task_handler()) {
          };
        }
        // Clear any ongoing touch pressed events
        // Without this LVGL gets stuck in the pressed state and will keep refreshing the
        // display activity timer causing the screen to never sleep after timeout
        lvgl.ClearTouchState();
        if (msg == Messages::GoToAOD) {
          lcd.LowPowerOn();
          // Record idle entry time
          alwaysOnFrameCount = 0;
          alwaysOnStartTime = xTaskGetTickCount();
          PushMessageToSystemTask(Pinetime::System::Messages::OnDisplayTaskAOD);
          state = States::AOD;
        } else {
          lcd.Sleep();
          PushMessageToSystemTask(Pinetime::System::Messages::OnDisplayTaskSleeping);
          state = States::Idle;
        }
        break;
      case Messages::NotifyDeviceActivity:
        lv_disp_trig_activity(nullptr);
        break;
      case Messages::GoToRunning:
        // If SystemTask is sleeping, the GoToRunning message is old
        // and must be ignored. Otherwise DisplayApp will use SPI
        // that is powered down and cause bad behaviour
        if (state == States::Running || systemTask->IsSleeping()) {
          break;
        }
        if (state == States::AOD) {
          lcd.LowPowerOff();
        } else {
          lcd.Wakeup();
        }
        lv_disp_trig_activity(nullptr);
        ApplyBrightness();
        state = States::Running;
        break;
      case Messages::UpdateBleConnection:
        // Only used for recovery firmware
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
      case Messages::Chime:
        LoadNewScreen(Apps::Clock, DisplayApp::FullRefreshDirections::None);
        motorController.RunForDuration(35);
        break;
      case Messages::OnChargingEvent:
        motorController.RunForDuration(15);
        break;
    }
  }

  if (state == States::Running && touchHandler.IsTouching()) {
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
                                                                 alarmController,
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
                                                               bleController,
                                                               alarmController);
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
    case Apps::SettingHeartRate:
      currentScreen = std::make_unique<Screens::SettingHeartRate>(settingsController);
      break;
    case Apps::SettingDisplay:
      currentScreen = std::make_unique<Screens::SettingDisplay>(settingsController);
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
