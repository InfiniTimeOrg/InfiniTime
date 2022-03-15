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

          lv_obj_t * btnNextList;
          lv_obj_t * btnPrevList;
          lv_obj_t * btnNextTile;
          lv_obj_t * btnPrevTile;
          lv_obj_t * btnReset;
          lv_obj_t * listColor;
          lv_obj_t * tileColor;
          lv_obj_t * backgroundLabel;
      };
    }
  }
}