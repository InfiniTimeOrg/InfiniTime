#pragma once

#include <lvgl/lvgl.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"

namespace Pinetime {
  namespace Controllers {
    class Settings;
    class Battery;
    class MotionController;
  }

  namespace Applications {
    namespace Screens {

      class WatchFaceHorizon : public Screen {
      public:
        WatchFaceHorizon(DisplayApp* app,
                         Controllers::DateTime& dateTimeController,
                         Controllers::Battery& batteryController,
                         Controllers::Settings& settingsController,
                         Controllers::MotionController& motionController,
                         Controllers::FS& fs);

        ~WatchFaceHorizon() override;

        void Refresh() override;

        static bool IsAvailable(Pinetime::Controllers::FS& filesystem);

      private:
        char displayedChar[5] {};

        uint16_t currentYear = 1970;
        Pinetime::Controllers::DateTime::Months currentMonth = Pinetime::Controllers::DateTime::Months::Unknown;
        Pinetime::Controllers::DateTime::Days currentDayOfWeek = Pinetime::Controllers::DateTime::Days::Unknown;
        uint8_t currentDay = 0;

        DirtyValue<uint8_t> batteryPercentRemaining {};
        DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> currentDateTime {};
        DirtyValue<bool> motionSensorOk {};
        DirtyValue<uint32_t> stepCount {};

        lv_obj_t* background;

        lv_obj_t* hourLines[24];
        lv_style_t hourLineStyles[24];
        lv_point_t hourLinePoints[24][2];

        lv_obj_t* labelHourFirstDigit;
        lv_obj_t* labelHourSecondDigit;

        lv_obj_t* labelMinutesFirstDigit;
        lv_obj_t* labelMinutesSecondDigit;

        lv_obj_t* lineBatteryFg;
        lv_obj_t* lineBatteryBg;
        lv_style_t lineBatteryFgStyle;
        lv_style_t lineBatteryBgStyle;
        lv_point_t lineBatteryFgPoints[2];
        lv_point_t lineBatteryBgPoints[2];

        lv_obj_t* labelDayOfWeek;
        lv_obj_t* labelMonth;
        lv_obj_t* labelDate;

        lv_obj_t* stepValue;

        Controllers::DateTime& dateTimeController;
        Controllers::Battery& batteryController;
        Controllers::Settings& settingsController;
        Controllers::MotionController& motionController;

        lv_task_t* taskRefresh;
        lv_font_t* font_pinecone_28 = nullptr;
        lv_font_t* font_pinecone_70 = nullptr;

        int hourlyColors[24] = {0x353f76, 0x415587, 0x4e6a98, 0x5a80a9, 0x6696ba, 0x7fa5b1, 0x98b5a7, 0xb0c49e,
                                0xc9d494, 0xe2e38b, 0xe3d780, 0xe4ca75, 0xe5be69, 0xe6b15e, 0xe7a553, 0xd29357,
                                0xbd815b, 0xa86f60, 0x935d64, 0x7e4b68, 0x6d4467, 0x4b3766, 0x3a3066, 0x292965};
      };
    }
  }
}
