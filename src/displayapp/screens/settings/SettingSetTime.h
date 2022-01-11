#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/datetime/DateTimeController.h"
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class SettingSetTime : public Screen {
      public:
        SettingSetTime(DisplayApp* app,
                       Pinetime::Controllers::DateTime& dateTimeController,
                       Pinetime::Controllers::Settings& settingsController);
        ~SettingSetTime() override;

        void HandleButtonPress(lv_obj_t* object, lv_event_t event);

      private:
        Controllers::DateTime& dateTimeController;
        Controllers::Settings& settingsController;

        int hoursValue;
        int minutesValue;
        lv_obj_t* lblHours;
        lv_obj_t* lblMinutes;
        lv_obj_t* lblampm;
        lv_obj_t* btnHoursPlus;
        lv_obj_t* btnHoursMinus;
        lv_obj_t* btnMinutesPlus;
        lv_obj_t* btnMinutesMinus;
        lv_obj_t* btnSetTime;

        void setHourLabels(int time24H) {
          switch (time24H) {
            if (settingsController.GetClockType() == Controllers::Settings::ClockType::H12) {
              case 0:
                lv_label_set_text_static(lblHours, "12");
                lv_label_set_text_static(lblampm, "AM");
                break;
              case 1 ... 11:
                lv_label_set_text_fmt(lblHours, "%02d", time24H);
                lv_label_set_text_static(lblampm, "AM");
                break;
              case 12:
                lv_label_set_text_static(lblHours, "12");
                lv_label_set_text_static(lblampm, "PM");
                break;
              case 13 ... 23:
                lv_label_set_text_fmt(lblHours, "%02d", time24H - 12);
                lv_label_set_text_static(lblampm, "PM");
                break;
            } else {
              lv_label_set_text_fmt(lblHours, "%02d", hoursValue);
            }
          }
        }
      };
    }
  }
}
