#pragma once

#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingWatchFace : public Screen {
      public:
        SettingWatchFace(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingWatchFace() override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        const std::array<std::string, 3> options = {" Digital face", " Analog face", " PineTimeStyle"};
        Controllers::Settings& settingsController;

        lv_obj_t* cbOption[2];
      };
    }
  }
}
