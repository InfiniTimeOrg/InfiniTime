#pragma once
#include <FreeRTOS.h>
#include <date/date.h>
#include <queue.h>
#include <task.h>
#include <memory>
#include "Apps.h"
#include "LittleVgl.h"
#include "TouchEvents.h"
#include "components/brightness/BrightnessController.h"
#include "components/motor/MotorController.h"
#include "components/firmwarevalidator/FirmwareValidator.h"
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include "components/timer/TimerController.h"
#include "Messages.h"

namespace Pinetime {

  namespace Drivers {
    class St7789;
    class Cst816S;
    class WatchdogView;
  }
  namespace Controllers {
    class Settings;
    class Battery;
    class Ble;
    class DateTime;
    class NotificationManager;
    class HeartRateController;
    class MotionController;
  }

  namespace System {
    class SystemTask;
  };
  namespace Applications {
    class DisplayApp {
    public:
      enum class States { Idle, Running };
      enum class FullRefreshDirections { None, Up, Down, Left, Right, LeftAnim, RightAnim };
      enum class TouchModes { Gestures, Polling };

      DisplayApp(Drivers::St7789& lcd,
                 Components::LittleVgl& lvgl,
                 Drivers::Cst816S&,
                 Controllers::Battery& batteryController,
                 Controllers::Ble& bleController,
                 Controllers::DateTime& dateTimeController,
                 Drivers::WatchdogView& watchdog,
                 System::SystemTask& systemTask,
                 Pinetime::Controllers::NotificationManager& notificationManager,
                 Pinetime::Controllers::HeartRateController& heartRateController,
                 Controllers::Settings& settingsController,
                 Pinetime::Controllers::MotorController& motorController,
                 Pinetime::Controllers::MotionController& motionController,
                 Pinetime::Controllers::TimerController& timerController);
      void Start();
      void PushMessage(Display::Messages msg);

      void StartApp(Apps app, DisplayApp::FullRefreshDirections direction);

      void SetFullRefresh(FullRefreshDirections direction);
      void SetTouchMode(TouchModes mode);

    private:
      Pinetime::Drivers::St7789& lcd;
      Pinetime::Components::LittleVgl& lvgl;
      Pinetime::Drivers::Cst816S& touchPanel;
      Pinetime::Controllers::Battery& batteryController;
      Pinetime::Controllers::Ble& bleController;
      Pinetime::Controllers::DateTime& dateTimeController;
      Pinetime::Drivers::WatchdogView& watchdog;
      Pinetime::System::SystemTask& systemTask;
      Pinetime::Controllers::NotificationManager& notificationManager;
      Pinetime::Controllers::HeartRateController& heartRateController;
      Pinetime::Controllers::Settings& settingsController;
      Pinetime::Controllers::MotorController& motorController;
      Pinetime::Controllers::MotionController& motionController;
      Pinetime::Controllers::TimerController& timerController;

      Pinetime::Controllers::FirmwareValidator validator;
      Controllers::BrightnessController brightnessController;

      TaskHandle_t taskHandle;

      States state = States::Running;
      QueueHandle_t msgQueue;

      static constexpr uint8_t queueSize = 10;
      static constexpr uint8_t itemSize = 1;

      std::unique_ptr<Screens::Screen> currentScreen;

      Apps currentApp = Apps::None;
      Apps returnToApp = Apps::None;
      FullRefreshDirections returnDirection = FullRefreshDirections::None;
      TouchEvents returnTouchEvent = TouchEvents::None;

      TouchModes touchMode = TouchModes::Gestures;

      TouchEvents OnTouchEvent();
      void RunningState();
      void IdleState();
      static void Process(void* instance);
      void InitHw();
      void Refresh();
      void ReturnApp(Apps app, DisplayApp::FullRefreshDirections direction, TouchEvents touchEvent);
      void LoadApp(Apps app, DisplayApp::FullRefreshDirections direction);
    };
  }
}
