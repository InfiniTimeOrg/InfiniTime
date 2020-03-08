#pragma once
#include <FreeRTOS.h>
#include <task.h>
#include <drivers/St7789.h>
#include <drivers/SpiMaster.h>
#include <Components/Gfx/Gfx.h>
#include <bits/unique_ptr.h>
#include <queue.h>
#include <Components/Battery/BatteryController.h>
#include <Components/Ble/BleController.h>
#include <Components/DateTime/DateTimeController.h>
#include "Fonts/lcdfont14.h"
#include "../drivers/Cst816s.h"
#include "LittleVgl.h"
#include <date/date.h>
#include <DisplayApp/Screens/Clock.h>


namespace Pinetime {
  namespace System {
    class SystemTask;
  };
  namespace Applications {
    class DisplayApp {
      public:
        enum class States {Idle, Running};
        enum class Messages : uint8_t {GoToSleep, GoToRunning, UpdateDateTime, UpdateBleConnection, UpdateBatteryLevel, TouchEvent, SwitchScreen,ButtonPushed} ;
        DisplayApp(Pinetime::Drivers::St7789& lcd,
                   Pinetime::Components::LittleVgl& lvgl,
                   Pinetime::Drivers::Cst816S&,
                   Controllers::Battery &batteryController,
                   Controllers::Ble &bleController,
                   Controllers::DateTime& dateTimeController,
                   Pinetime::System::SystemTask& systemTask);
        void Start();
        void PushMessage(Messages msg);

        enum class Apps {None, Launcher, Clock, Test, Meter, Gauge};
        void StartApp(Apps app);

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

        Pinetime::Drivers::Cst816S& touchPanel;
        void OnTouchEvent();

        std::unique_ptr<Screens::Screen> currentScreen;
        static constexpr uint8_t pinLcdBacklight1 = 14;
        static constexpr uint8_t pinLcdBacklight2 = 22;
        static constexpr uint8_t pinLcdBacklight3 = 23;

        bool isClock = true;

        Pinetime::System::SystemTask& systemTask;
        Apps nextApp = Apps::None;
    };
  }
}


