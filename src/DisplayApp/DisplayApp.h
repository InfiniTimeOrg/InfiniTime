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
#include "lcdfont14.h"

extern const FONT_INFO lCD_70ptFontInfo;

namespace Pinetime {
  namespace Applications {
    class DisplayApp {
      public:
        enum class States {Idle, Running};
        enum class Messages : uint8_t {GoToSleep, GoToRunning, UpdateDateTime, UpdateBleConnection, UpdateBatteryLevel} ;
        DisplayApp(Controllers::Battery &batteryController,
                   Controllers::Ble &bleController,
                   Controllers::DateTime& dateTimeController);
        void Start();
        void PushMessage(Messages msg);

      private:
        TaskHandle_t taskHandle;
        static void Process(void* instance);
        void InitHw();
        Pinetime::Drivers::SpiMaster spi;
        std::unique_ptr<Drivers::St7789> lcd;
        std::unique_ptr<Components::Gfx> gfx;
        const FONT_INFO largeFont {lCD_70ptFontInfo.height, lCD_70ptFontInfo.startChar, lCD_70ptFontInfo.endChar, lCD_70ptFontInfo.spacePixels, lCD_70ptFontInfo.charInfo, lCD_70ptFontInfo.data};
        const FONT_INFO smallFont {lCD_14ptFontInfo.height, lCD_14ptFontInfo.startChar, lCD_14ptFontInfo.endChar, lCD_14ptFontInfo.spacePixels, lCD_14ptFontInfo.charInfo, lCD_14ptFontInfo.data};
        void Refresh();

        static const char* MonthToString(Pinetime::Controllers::DateTime::Months month);
        static const char* DayOfWeekToString(Pinetime::Controllers::DateTime::Days dayOfWeek);
        uint8_t seconds = 0;
        uint8_t minutes = 0;
        uint8_t hours = 0;
        char currentChar[4];
        uint32_t deltaSeconds = 0;

        States state = States::Running;
        void RunningState();
        void IdleState();
        QueueHandle_t msgQueue;

        static constexpr uint8_t queueSize = 10;
        static constexpr uint8_t itemSize = 1;

        Pinetime::Controllers::Battery &batteryController;
        Pinetime::Controllers::Ble &bleController;
        Pinetime::Controllers::DateTime& dateTimeController;
        bool bleConnectionUpdated = false;
        bool batteryLevelUpdated = false;

        static char const *DaysString[];

        static char const *MonthsString[];

        bool dateUpdated = false;

    };
  }
}


