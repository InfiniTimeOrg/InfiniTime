#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>

#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingTimeFormat : public Screen {
      public:
        SettingTimeFormat(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingTimeFormat() override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        struct Option {
          Controllers::Settings::ClockType clockType;
          const char* name;
        };

        static constexpr std::array<Option, 2> options = {{
          {Controllers::Settings::ClockType::H12, "12-hour"},
          {Controllers::Settings::ClockType::H24, "24-hour"},
        }};
        Controllers::Settings& settingsController;
        lv_obj_t* cbOption[options.size()];
      };
    }
  }
}
