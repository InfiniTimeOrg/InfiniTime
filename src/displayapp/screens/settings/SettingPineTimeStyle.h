#pragma once

#include "components/settings/Settings.h"
#include "displayapp/screens/PineTimeStyleBase.h"
#include "displayapp/screens/Screen.h"
#include <cstdint>
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class SettingPineTimeStyle : public PineTimeStyleBase {
      public:
        SettingPineTimeStyle(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingPineTimeStyle() override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        Controllers::Settings& settingsController;

        Pinetime::Controllers::Settings::Colors GetNext(Controllers::Settings::Colors color);
        Pinetime::Controllers::Settings::Colors GetPrevious(Controllers::Settings::Colors color);

        lv_obj_t* btnNextBG;
        lv_obj_t* btnNextBar;
        lv_obj_t* btnNextTime;
        lv_obj_t* btnPrevBG;
        lv_obj_t* btnPrevBar;
        lv_obj_t* btnPrevTime;
        lv_obj_t* btnRandom;
        lv_obj_t* btnReset;
      };
    }
  }
}
