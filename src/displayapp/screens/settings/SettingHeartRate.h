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
        const int intervalInSeconds;
        const char* name;
      };

      class SettingHeartRate : public Screen {
      public:
        SettingHeartRate(Pinetime::Controllers::Settings& settings);
        ~SettingHeartRate() override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        Pinetime::Controllers::Settings& settingsController;

        static constexpr std::array<Option, 8> options = {{
          {-1, " Off"},
          {0, "Cont"},
          {15, " 15s"},
          {30, " 30s"},
          {60, "  1m"},
          {5 * 60, "  5m"},
          {10 * 60, " 10m"},
          {30 * 60, " 30m"},
        }};

        lv_obj_t* cbOption[options.size()];
      };
    }
  }
}
