#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>

#include "components/settings/Settings.h"
#include "displayapp/screens/ScreenList.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/CheckboxList.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      struct Option {
        const uint32_t interval;
        const char* name;
      };

      class SettingHeartRate : public Screen {
      public:
        SettingHeartRate(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settings);
        ~SettingHeartRate() override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        DisplayApp* app;
        Pinetime::Controllers::Settings& settingsController;

        static constexpr std::array<Option, 8> options = {{
          {0, "Off"},
          {10, "10s"},
          {30, "30s"},
          {60, " 1m"},
          {5 * 60, " 5m"},
          {10 * 60, "10m"},
          {30 * 60, "30m"},
          {60 * 60, " 1h"},
        }};

        lv_obj_t* cbOption[options.size()];
      };
    }
  }
}
