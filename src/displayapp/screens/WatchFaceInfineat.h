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
    class Ble;
    class NotificationManager;
    class MotionController;
  }

  namespace Applications {
    namespace Screens {

      class WatchFaceInfineat : public Screen {
      public:
        WatchFaceInfineat(DisplayApp* app,
                          Controllers::DateTime& dateTimeController,
                          Controllers::Battery& batteryController,
                          Controllers::Ble& bleController,
                          Controllers::NotificationManager& notificationManager,
                          Controllers::Settings& settingsController,
                          Controllers::MotionController& motionController,
                          Controllers::FS& fs);

        ~WatchFaceInfineat() override;

        bool OnTouchEvent(TouchEvents event) override;
        bool OnButtonPushed() override;
        void UpdateSelected(lv_obj_t* object, lv_event_t event);
        void CloseMenu();

        void Refresh() override;

        static bool IsAvailable(Pinetime::Controllers::FS& filesystem);

      private:
        char displayedChar[5] {};

        uint16_t currentYear = 1970;
        Pinetime::Controllers::DateTime::Months currentMonth = Pinetime::Controllers::DateTime::Months::Unknown;
        Pinetime::Controllers::DateTime::Days currentDayOfWeek = Pinetime::Controllers::DateTime::Days::Unknown;
        uint8_t currentDay = 0;
        uint32_t savedTick = 0;
        uint8_t chargingBatteryPercent = 101; // not a mistake ;)

        DirtyValue<uint8_t> batteryPercentRemaining {};
        DirtyValue<bool> isCharging {};
        DirtyValue<bool> bleState {};
        DirtyValue<bool> bleRadioEnabled {};
        DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> currentDateTime {};
        DirtyValue<bool> motionSensorOk {};
        DirtyValue<uint32_t> stepCount {};
        DirtyValue<bool> notificationState {};

        lv_obj_t* background;

        // Lines making up the side cover
        lv_obj_t* line0;
        lv_obj_t* line1;
        lv_obj_t* line2;
        lv_obj_t* line3;
        lv_obj_t* line4;
        lv_obj_t* line5;
        lv_obj_t* line6;
        lv_obj_t* line7;
        lv_obj_t* line8;
        lv_obj_t* lineBattery;

        lv_style_t line0Style;
        lv_style_t line1Style;
        lv_style_t line2Style;
        lv_style_t line3Style;
        lv_style_t line4Style;
        lv_style_t line5Style;
        lv_style_t line6Style;
        lv_style_t line7Style;
        lv_style_t line8Style;
        lv_style_t lineBatteryStyle;

        lv_point_t line0Points[2];
        lv_point_t line1Points[2];
        lv_point_t line2Points[2];
        lv_point_t line3Points[2];
        lv_point_t line4Points[2];
        lv_point_t line5Points[2];
        lv_point_t line6Points[2];
        lv_point_t line7Points[2];
        lv_point_t line8Points[2];
        lv_point_t lineBatteryPoints[2];

        lv_obj_t* logoPine;

        lv_obj_t* timeContainer;
        lv_obj_t* labelHour;
        lv_obj_t* labelMinutes;
        lv_obj_t* labelTimeAmPm;
        lv_obj_t* dateContainer;
        lv_obj_t* labelDate;
        lv_obj_t* bleIcon;
        lv_obj_t* stepIcon;
        lv_obj_t* stepValue;
        lv_obj_t* notificationIcon;
        lv_obj_t* btnClose;
        lv_obj_t* btnNextColor;
        lv_obj_t* btnToggleCover;
        lv_obj_t* btnPrevColor;
        lv_obj_t* btnSettings;
        lv_obj_t* labelBtnSettings;
        lv_obj_t* lblToggle;

        static constexpr int nLines = 9;
        static constexpr int nColors = 7; // must match number of colors in InfineatColors
        struct InfineatColors {
          int orange[nLines] = {0xfd872b, 0xdb3316, 0x6f1000, 0xfd7a0a, 0xffffff, 0xffffff, 0xffffff, 0xe85102, 0xea1c00};
          int blue[nLines] = {0xe7f8ff, 0x2232d0, 0x182a8b, 0xe7f8ff, 0xffffff, 0xffffff, 0xffffff, 0x5991ff, 0x1636ff};
          int green[nLines] = {0xb8ff9b, 0x088608, 0x004a00, 0xb8ff9b, 0xffffff, 0xffffff, 0xffffff, 0x62d515, 0x007400};
          int rainbow[nLines] = {0x2da400, 0xac09c4, 0xfe0303, 0x0d57ff, 0xffffff, 0xffffff, 0xffffff, 0xe0b900, 0xe85102};
          int gray[nLines] = {0xeeeeee, 0x98959b, 0x191919, 0xeeeeee, 0xffffff, 0xffffff, 0xffffff, 0x919191, 0x3a3a3a};
          int nordBlue[nLines] = {0xc3daf2, 0x4d78ce, 0x153451, 0xc3daf2, 0xffffff, 0xffffff, 0xffffff, 0x5d8ad2, 0x21518a};
          int nordGreen[nLines] = {0xd5f0e9, 0x238373, 0x1d413f, 0xd5f0e9, 0xffffff, 0xffffff, 0xffffff, 0x2fb8a2, 0x11705a};
        } infineatColors;

        Controllers::DateTime& dateTimeController;
        Controllers::Battery& batteryController;
        Controllers::Ble& bleController;
        Controllers::NotificationManager& notificationManager;
        Controllers::Settings& settingsController;
        Controllers::MotionController& motionController;

        void SetBatteryLevel(uint8_t batteryPercent);
        void ToggleBatteryIndicatorColor(bool showSideCover);

        lv_task_t* taskRefresh;
        lv_font_t* font_teko = nullptr;
        lv_font_t* font_bebas = nullptr;
      };
    }
  }
}
