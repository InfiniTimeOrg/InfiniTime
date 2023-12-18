#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/BatteryIcon.h"
#include "components/datetime/DateTimeController.h"
#include "components/ble/BleController.h"
#include "utility/DirtyValue.h"

namespace Pinetime {
  namespace Controllers {
    class Settings;
    class Battery;
    class Ble;
    class NotificationManager;
    class HeartRateController;
    class MotionController;
  }

  namespace Applications {
    namespace Screens {

      /*
        WatchFacePineBoy is a face strongly inspired the by the Fallout's Pip Boy
        gadget.

        The watch face tries to display a small Pip Boy image which has to be
        included in the flash memory under /imgs/pine-boy-icon.bin. If this file
        is not present then it shows a simple rectangle instead. A good image would
        be with size of 42x82 pixels.
      */
      class WatchFacePineBoy : public Screen {
      public:
        WatchFacePineBoy(Controllers::DateTime& dateTimeController,
                         const Controllers::Battery& batteryController,
                         const Controllers::Ble& bleController,
                         Controllers::NotificationManager& notificationManager,
                         Controllers::Settings& settingsController,
                         Controllers::HeartRateController& heartRateController,
                         Controllers::MotionController& motionController,
                         Controllers::FS& filesystem);
        ~WatchFacePineBoy() override;

        void Refresh() override;

      private:

        void initFonts();
        void drawTabsMenu();
        void drawIndicators();
        void drawTime();
        void drawBoy(Controllers::FS& filesystem);

        lv_font_t* fontBig = nullptr;
        lv_font_t* fontNormal = nullptr;
        lv_font_t* fontSmall = nullptr;

        const int timeOffset = 40;
        const int ampmOFfset = timeOffset + 60;
        const lv_color_t primaryColor = LV_COLOR_MAKE(0x17, 0xca, 0x1a);
        const lv_color_t secondaryColor = LV_COLOR_MAKE(0x44, 0x87, 0x25);
        const lv_color_t attentionColor = LV_COLOR_MAKE(0xfe, 0x14, 0xe2);
        const lv_color_t warningsColor = LV_COLOR_MAKE(0xfe, 0x14, 0x5a);

        uint8_t displayedHour = -1;
        uint8_t displayedMinute = -1;

        Screens::BatteryIcon batteryIcon;
        Utility::DirtyValue<uint8_t> batteryPercentRemaining {};
        Utility::DirtyValue<bool> powerPresent {};
        Utility::DirtyValue<bool> bleState {};
        Utility::DirtyValue<bool> bleRadioEnabled {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>> currentDateTime {};
        Utility::DirtyValue<uint32_t> stepCount {};
        Utility::DirtyValue<uint8_t> heartbeat {};
        Utility::DirtyValue<bool> heartbeatRunning {};
        Utility::DirtyValue<bool> notificationState {};
        using days = std::chrono::duration<int32_t, std::ratio<86400>>; // TODO: days is standard in c++20
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, days>> currentDate;

        lv_obj_t* label_time;
        lv_obj_t* label_time_ampm;
        lv_obj_t* heartbeatIcon;
        lv_obj_t* heartbeatValue;
        lv_obj_t* boyImage;

        // Stats Icons
        lv_obj_t* indicators;
        lv_obj_t* batteryBorder;
        lv_obj_t* calBorder;
        lv_obj_t* calIcon;
        lv_obj_t* stepBorder;
        lv_obj_t* stepIcon;
        lv_obj_t* btBorder;
        lv_obj_t* btIcon;

        // Stats Values
        lv_obj_t* batteryValue;
        lv_obj_t* stepValue;
        lv_obj_t* btValue;
        lv_obj_t* calValue;

        // tabs menu
        lv_obj_t* tabsMenu;
        lv_obj_t* topUnderlineLeft;
        lv_obj_t* topUnderlineRight;
        lv_obj_t* bottomLine;
        lv_style_t menuLinesStyle;
        lv_obj_t* tabsLabelTime;
        lv_obj_t* tabsLabelComm;
        lv_obj_t* tabsLabelQuick;
        lv_obj_t* tabsLabelApps;

        Controllers::DateTime& dateTimeController;
        const Pinetime::Controllers::Battery& batteryController;
        const Controllers::Ble& bleController;
        Controllers::NotificationManager& notificationManager;
        Controllers::Settings& settingsController;
        Controllers::HeartRateController& heartRateController;
        Controllers::MotionController& motionController;

        lv_task_t* taskRefresh;
      };
    }
  }
}
