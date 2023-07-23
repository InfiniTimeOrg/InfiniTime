#pragma once
#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>
#include <memory>
#include <systemtask/Messages.h>
#include "displayapp/Apps.h"
#include "displayapp/LittleVgl.h"
#include "displayapp/TouchEvents.h"
#include "components/brightness/BrightnessController.h"
#include "components/motor/MotorController.h"
#include "components/firmwarevalidator/FirmwareValidator.h"
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/AppController.h"
#include "components/timer/Timer.h"
#include "components/alarm/AlarmController.h"
#include "touchhandler/TouchHandler.h"

#include "displayapp/Messages.h"
#include "BootErrors.h"

#include "utility/StaticStack.h"

namespace Pinetime {

  namespace Drivers {
    class St7789;
    class Cst816S;
    class Watchdog;
  }

  namespace Controllers {
    class Settings;
    class Battery;
    class Ble;
    class DateTime;
    class NotificationManager;
    class HeartRateController;
    class MotionController;
    class TouchHandler;
  }

  namespace System {
    class SystemTask;
  };

  namespace Applications {
    class DisplayApp {
    public:
      enum class States { Idle, Running };
      enum class FullRefreshDirections { None, Up, Down, Left, Right, LeftAnim, RightAnim };

      DisplayApp(Drivers::St7789& lcd,
                 const Drivers::Cst816S&,
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
                 Pinetime::Controllers::FS& filesystem);
      void Start(System::BootErrors error);
      void PushMessage(Display::Messages msg);

      void StartApp(uint8_t app, DisplayApp::FullRefreshDirections direction);

      void SetFullRefresh(FullRefreshDirections direction);

      void Register(Pinetime::System::SystemTask* systemTask);

    private:
      Pinetime::Drivers::St7789& lcd;
      const Pinetime::Drivers::Cst816S& touchPanel;
      const Pinetime::Controllers::Battery& batteryController;
      const Pinetime::Controllers::Ble& bleController;
      Pinetime::Controllers::DateTime& dateTimeController;
      const Pinetime::Drivers::Watchdog& watchdog;
      Pinetime::System::SystemTask* systemTask = nullptr;
      Pinetime::Controllers::NotificationManager& notificationManager;
      Pinetime::Controllers::HeartRateController& heartRateController;
      Pinetime::Controllers::Settings& settingsController;
      Pinetime::Controllers::MotorController& motorController;
      Pinetime::Controllers::MotionController& motionController;
      Pinetime::Controllers::AlarmController& alarmController;
      Pinetime::Controllers::BrightnessController& brightnessController;
      Pinetime::Controllers::TouchHandler& touchHandler;
      Pinetime::Controllers::FS& filesystem;
      Pinetime::Applications::AppController appController;

      Pinetime::Controllers::FirmwareValidator validator;
      Pinetime::Components::LittleVgl lvgl;
      Pinetime::Controllers::Timer timer;

      TaskHandle_t taskHandle;

      States state = States::Running;
      QueueHandle_t msgQueue;

      static constexpr uint8_t queueSize = 10;
      static constexpr uint8_t itemSize = 1;

      std::unique_ptr<Screens::Screen> currentScreen;

      uint8_t currentApp = static_cast<uint8_t>(Apps::None);
      uint8_t returnToApp = static_cast<uint8_t>(Apps::None);
      FullRefreshDirections returnDirection = FullRefreshDirections::None;
      TouchEvents returnTouchEvent = TouchEvents::None;

      TouchEvents GetGesture();
      static void Process(void* instance);
      void InitHw();
      void Refresh();
      void LoadNewScreen(uint8_t app, DisplayApp::FullRefreshDirections direction);
      void LoadScreen(uint8_t app, DisplayApp::FullRefreshDirections direction);
      void PushMessageToSystemTask(Pinetime::System::Messages message);

      uint8_t nextApp = static_cast<uint8_t>(Apps::None);
      DisplayApp::FullRefreshDirections nextDirection;
      System::BootErrors bootError;
      void ApplyBrightness();

      static constexpr size_t returnAppStackSize = 10;
      Utility::StaticStack<uint8_t, returnAppStackSize> returnAppStack;
      Utility::StaticStack<FullRefreshDirections, returnAppStackSize> appStackDirections;

      bool isDimmed = false;
    };
  }
}
