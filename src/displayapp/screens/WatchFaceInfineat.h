#pragma once

#include <lvgl/lvgl.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "utility/DirtyValue.h"

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
        static constexpr int nLines = 9;
        WatchFaceInfineat(Controllers::DateTime& dateTimeController,
                          const Controllers::Battery& batteryController,
                          const Controllers::Ble& bleController,
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
        uint32_t savedTick = 0;
        uint8_t chargingBatteryPercent = 101; // not a mistake ;)

        Utility::DirtyValue<uint8_t> batteryPercentRemaining {};
        Utility::DirtyValue<bool> isCharging {};
        Utility::DirtyValue<bool> bleState {};
        Utility::DirtyValue<bool> bleRadioEnabled {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>> currentDateTime {};
        Utility::DirtyValue<uint32_t> stepCount {};
        Utility::DirtyValue<bool> notificationState {};
        using days = std::chrono::duration<int32_t, std::ratio<86400>>; // TODO: days is standard in c++20
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, days>> currentDate;

        // Lines making up the side cover
        lv_obj_t* lineBattery;

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

        lv_obj_t* lines[nLines];

        Controllers::DateTime& dateTimeController;
        const Controllers::Battery& batteryController;
        const Controllers::Ble& bleController;
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
