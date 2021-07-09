#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/datetime/DateTimeController.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingSetTime : public Screen{
        public:
          SettingSetTime(DisplayApp* app, Pinetime::Controllers::DateTime &dateTimeController);
          ~SettingSetTime() override;

          bool Refresh() override;
          void HandleButtonPress(lv_obj_t *object, lv_event_t event);
         
        private:          

          Controllers::DateTime& dateTimeController;

          int hoursValue;
          int minutesValue;
          lv_obj_t * lblHours;
          lv_obj_t * lblMinutes;
          lv_obj_t * btnHoursPlus;
          lv_obj_t * btnHoursMinus;
          lv_obj_t * btnMinutesPlus;
          lv_obj_t * btnMinutesMinus;
          lv_obj_t * btnSetTime;
          
      };
    }
  }
}
