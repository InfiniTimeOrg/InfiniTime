#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>

#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingAirplaneMode : public Screen {
      public:
        SettingAirplaneMode(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingAirplaneMode() override;

        void OnAirplaneModeEnabled(lv_obj_t* object, lv_event_t event);
        void OnAirplaneModeDisabled(lv_obj_t* object, lv_event_t event);

      private:
        Controllers::Settings& settingsController;
        lv_obj_t* cbEnabled;
        lv_obj_t* cbDisabled;
        bool priorMode;
      };
    }
  }
}
