#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingPineTimeStyle : public Screen{
        public:
          SettingPineTimeStyle(DisplayApp* app, Pinetime::Controllers::Settings &settingsController);
          ~SettingPineTimeStyle() override;

          void UpdateSelected(lv_obj_t *object, lv_event_t *event);
         
        private:          
          Controllers::Settings& settingsController;

          Pinetime::Controllers::Settings::Colors GetNext(Controllers::Settings::Colors color);
          Pinetime::Controllers::Settings::Colors GetPrevious(Controllers::Settings::Colors color);

          lv_obj_t * btnNextTime, * txtNextTime;
          lv_obj_t * btnPrevTime, * txtPrevTime;
          lv_obj_t * btnNextBar, * txtNextBar;
          lv_obj_t * btnPrevBar, * txtPrevBar;
          lv_obj_t * btnNextBG, * txtNextBG;
          lv_obj_t * btnPrevBG, * txtPrevBG;
          lv_obj_t * btnReset, * txtReset;
          lv_obj_t * btnRandom, * txtRandom;
          lv_obj_t * timebar;
          lv_obj_t * sidebar;
          lv_obj_t * timeDD1;
          lv_obj_t * timeDD2;
          lv_obj_t * timeAMPM;
          lv_obj_t * dateDayOfWeek;
          lv_obj_t * dateDay;
          lv_obj_t * dateMonth;
          lv_obj_t * backgroundLabel;
          lv_obj_t * batteryIcon;
          lv_obj_t * bleIcon;
          lv_obj_t * calendarOuter;
          lv_obj_t * calendarInner;
          lv_obj_t * calendarBar1;
          lv_obj_t * calendarBar2;
          lv_obj_t * calendarCrossBar1;
          lv_obj_t * calendarCrossBar2;
          lv_obj_t * stepMeter;
          lv_meter_scale_t * stepScale;
          lv_meter_indicator_t * stepIndicator;
          lv_color_t needle_color;
      };
    }
  }
}
