#pragma once

#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingTimeFormat : public Screen {
      public:
        SettingTimeFormat(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingTimeFormat() override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        const std::array<std::string, 2> options = {" 12-hour", " 24-hour"};

        Controllers::Settings& settingsController;
        lv_obj_t* cbOption[2];
      };
    }
  }
}
