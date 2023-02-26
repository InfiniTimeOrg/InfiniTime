#pragma once

#include <displayapp/screens/BatteryIcon.h>
#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "components/ble/BleController.h"

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

      class WatchFaceCasioStyleG7710 : public Screen {
      public:
        WatchFaceCasioStyleG7710(Controllers::DateTime& dateTimeController,
                                 const Controllers::Battery& batteryController,
                                 const Controllers::Ble& bleController,
                                 Controllers::NotificationManager& notificatioManager,
                                 Controllers::Settings& settingsController,
                                 Controllers::HeartRateController& heartRateController,
                                 Controllers::MotionController& motionController,
                                 Controllers::FS& filesystem);
        ~WatchFaceCasioStyleG7710() override;

        void Refresh() override;

        static bool IsAvailable(Pinetime::Controllers::FS& filesystem);

      private:
        uint8_t displayedHour = -1;
        uint8_t displayedMinute = -1;

        uint16_t currentYear = 1970;
        Controllers::DateTime::Months currentMonth = Pinetime::Controllers::DateTime::Months::Unknown;
        Controllers::DateTime::Days currentDayOfWeek = Pinetime::Controllers::DateTime::Days::Unknown;
        uint8_t currentDay = 0;

        DirtyValue<uint8_t> batteryPercentRemaining {};
        DirtyValue<bool> powerPresent {};
        DirtyValue<bool> bleState {};
        DirtyValue<bool> bleRadioEnabled {};
        DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> currentDateTime {};
        DirtyValue<bool> motionSensorOk {};
        DirtyValue<uint32_t> stepCount {};
        DirtyValue<uint8_t> heartbeat {};
        DirtyValue<bool> heartbeatRunning {};
        DirtyValue<bool> notificationState {};

        lv_point_t line_icons_points[3] {{0, 5}, {117, 5}, {122, 0}};
        lv_point_t line_day_of_week_number_points[4] {{0, 0}, {100, 0}, {95, 95}, {0, 95}};
        lv_point_t line_day_of_year_points[3] {{0, 5}, {130, 5}, {135, 0}};
        lv_point_t line_date_points[3] {{0, 5}, {135, 5}, {140, 0}};
        lv_point_t line_time_points[3] {{0, 0}, {230, 0}, {235, 5}};

        lv_color_t color_text = lv_color_hex(0x98B69A);

        lv_style_t style_line;
        lv_style_t style_border;

        lv_obj_t* label_time;
        lv_obj_t* line_time;
        lv_obj_t* label_time_ampm;
        lv_obj_t* label_date;
        lv_obj_t* line_date;
        lv_obj_t* label_day_of_week;
        lv_obj_t* label_week_number;
        lv_obj_t* line_day_of_week_number;
        lv_obj_t* label_day_of_year;
        lv_obj_t* line_day_of_year;
        lv_obj_t* backgroundLabel;
        lv_obj_t* bleIcon;
        lv_obj_t* batteryPlug;
        lv_obj_t* label_battery_vallue;
        lv_obj_t* heartbeatIcon;
        lv_obj_t* heartbeatValue;
        lv_obj_t* stepIcon;
        lv_obj_t* stepValue;
        lv_obj_t* notificationIcon;
        lv_obj_t* line_icons;

        BatteryIcon batteryIcon;

        Controllers::DateTime& dateTimeController;
        const Controllers::Battery& batteryController;
        const Controllers::Ble& bleController;
        Controllers::NotificationManager& notificatioManager;
        Controllers::Settings& settingsController;
        Controllers::HeartRateController& heartRateController;
        Controllers::MotionController& motionController;

        lv_task_t* taskRefresh;
        lv_font_t* font_dot40 = nullptr;
        lv_font_t* font_segment40 = nullptr;
        lv_font_t* font_segment115 = nullptr;
      };
    }
  }
}
