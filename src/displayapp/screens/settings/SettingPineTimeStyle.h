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

          bool Refresh() override;
          void UpdateSelected(lv_obj_t *object, lv_event_t event);
         
        private:          
          Controllers::Settings& settingsController;

          lv_obj_t * btnNextTime;
          lv_obj_t * btnPrevTime;
          lv_obj_t * btnNextBar;
          lv_obj_t * btnPrevBar;
          lv_obj_t * btnNextBG;
          lv_obj_t * btnPrevBG;
          lv_obj_t * btnReset;
          lv_obj_t * timeColor;
          lv_obj_t * barColor;
          lv_obj_t * bgColor;
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
          lv_obj_t * stepGauge;
          lv_color_t needle_colors[1];
          lv_color_t pts_colors[17] = {LV_COLOR_WHITE, LV_COLOR_SILVER, LV_COLOR_GRAY, LV_COLOR_BLACK,
                                       LV_COLOR_RED, LV_COLOR_MAROON, LV_COLOR_YELLOW, LV_COLOR_OLIVE,
                                       LV_COLOR_LIME, LV_COLOR_GREEN, LV_COLOR_CYAN, LV_COLOR_TEAL,
                                       LV_COLOR_BLUE, LV_COLOR_NAVY, LV_COLOR_MAGENTA, LV_COLOR_PURPLE,
                                       LV_COLOR_ORANGE};
      };
    }
  }
}
