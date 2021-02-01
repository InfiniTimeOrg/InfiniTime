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
#include "components/firmwarevalidator/FirmwareValidator.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {

  namespace Drivers {
    class St7789;
    class Cst816S;
    class WatchdogView;
  }
  namespace Controllers {
    class Battery;
    class Ble;
    class DateTime;
    class NotificationManager;
    class HeartRateController;
  }

  namespace System {
    class SystemTask;
  };
  namespace Applications {
    class DisplayApp {
      public:
        enum class States {Idle, Running};
        enum class Messages : uint8_t {GoToSleep, GoToRunning, UpdateDateTime, UpdateBleConnection, UpdateBatteryLevel, TouchEvent, ButtonPushed,
            NewNotification, BleFirmwareUpdateStarted };

        enum class FullRefreshDirections { None, Up, Down };
        enum class TouchModes { Gestures, Polling };

        DisplayApp(Drivers::St7789 &lcd, Components::LittleVgl &lvgl, Drivers::Cst816S &,
                   Controllers::Battery &batteryController, Controllers::Ble &bleController,
                   Controllers::DateTime &dateTimeController, Drivers::WatchdogView &watchdog,
                   System::SystemTask &systemTask,
                   Pinetime::Controllers::NotificationManager& notificationManager,
                   Pinetime::Controllers::HeartRateController& heartRateController);
        void Start();
        void PushMessage(Messages msg);

        void StartApp(Apps app);

        void SetFullRefresh(FullRefreshDirections direction);
        void SetTouchMode(TouchModes mode);

      private:
        TaskHandle_t taskHandle;
        static void Process(void* instance);
        void InitHw();
        Pinetime::Drivers::St7789& lcd;
        Pinetime::Components::LittleVgl& lvgl;
        void Refresh();

        States state = States::Running;
        void RunningState();
        void IdleState();
        QueueHandle_t msgQueue;

        static constexpr uint8_t queueSize = 10;
        static constexpr uint8_t itemSize = 1;

        Pinetime::Controllers::Battery &batteryController;
        Pinetime::Controllers::Ble &bleController;
        Pinetime::Controllers::DateTime& dateTimeController;
        Pinetime::Drivers::WatchdogView& watchdog;

        Pinetime::Drivers::Cst816S& touchPanel;
        TouchEvents OnTouchEvent();

        std::unique_ptr<Screens::Screen> currentScreen;

        bool isClock = true;

        Pinetime::System::SystemTask& systemTask;
        Apps nextApp = Apps::None;
        bool onClockApp = false; // TODO find a better way to know that we should handle gestures and button differently for the Clock app.
        Controllers::BrightnessController brightnessController;
        Pinetime::Controllers::NotificationManager& notificationManager;
        Pinetime::Controllers::FirmwareValidator validator;
        TouchModes touchMode = TouchModes::Gestures;
        Pinetime::Controllers::HeartRateController& heartRateController;
    };
  }
}


