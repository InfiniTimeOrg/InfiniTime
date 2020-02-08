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
#include <date/date.h>
#include <DisplayApp/Screens/Clock.h>
#include <DisplayApp/Screens/Message.h>

extern const FONT_INFO lCD_70ptFontInfo;

namespace Pinetime {
  namespace Applications {
    class DisplayApp {
      public:
        enum class States {Idle, Running};
        enum class Messages : uint8_t {GoToSleep, GoToRunning, UpdateDateTime, UpdateBleConnection, UpdateBatteryLevel, TouchEvent} ;
        DisplayApp(Pinetime::Drivers::St7789& lcd,
                   Pinetime::Components::Gfx& gfx,
                   Pinetime::Drivers::Cst816S&,
                   Controllers::Battery &batteryController,
                   Controllers::Ble &bleController,
                   Controllers::DateTime& dateTimeController);
        void Start();
        void PushMessage(Messages msg);

      private:
        TaskHandle_t taskHandle;
        static void Process(void* instance);
        void InitHw();
        Pinetime::Drivers::St7789& lcd;
        Pinetime::Components::Gfx& gfx;
        const FONT_INFO largeFont {lCD_70ptFontInfo.height, lCD_70ptFontInfo.startChar, lCD_70ptFontInfo.endChar, lCD_70ptFontInfo.spacePixels, lCD_70ptFontInfo.charInfo, lCD_70ptFontInfo.data};
        const FONT_INFO smallFont {lCD_14ptFontInfo.height, lCD_14ptFontInfo.startChar, lCD_14ptFontInfo.endChar, lCD_14ptFontInfo.spacePixels, lCD_14ptFontInfo.charInfo, lCD_14ptFontInfo.data};
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

        Screens::Clock clockScreen;
        Screens::Message messageScreen;
        Screens::Screen* currentScreen = nullptr;
        static constexpr uint8_t pinLcdBacklight1 = 14;
        static constexpr uint8_t pinLcdBacklight2 = 22;
        static constexpr uint8_t pinLcdBacklight3 = 23;
    };
  }
}


