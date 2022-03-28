#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingColor : public Screen{
        public:
          SettingColor(DisplayApp* app, Pinetime::Controllers::Settings &settingsController);
          ~SettingColor() override;

          void UpdateSelected(lv_obj_t *object, lv_event_t event);
         
        private:          
          Controllers::Settings& settingsController;

          Pinetime::Controllers::Settings::Colors GetNext(Controllers::Settings::Colors color);
          Pinetime::Controllers::Settings::Colors GetPrevious(Controllers::Settings::Colors color);

          lv_obj_t * btnNextPrimary;
          lv_obj_t * btnPrevPrimary;
          lv_obj_t * btnNextSecondary;
          lv_obj_t * btnPrevSecondary;
          lv_obj_t * btnOpacity;
          lv_obj_t * btnReset;
          lv_obj_t * primaryColor;
          lv_obj_t * secondaryColor;
          lv_obj_t * labelPrimary;
          lv_obj_t * labelSecondary;
          // lv_obj_t * backgroundLabel;
      };
    }
  }
}