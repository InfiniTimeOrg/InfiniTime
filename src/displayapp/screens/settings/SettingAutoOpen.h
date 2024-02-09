#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>

#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingAutoOpen : public Screen {
      public:
        SettingAutoOpen(Pinetime::Controllers::Settings& settingsController);
        ~SettingAutoOpen() override;

        void UpdateSelected(lv_obj_t* object);

      private:
        struct Option {
          Controllers::Settings::AutoOpen app;
          const char* name;
        };

        Controllers::Settings& settingsController;
        static constexpr std::array<Option, 3> options = {{
          {Controllers::Settings::AutoOpen::Battery, "Battery"},
          {Controllers::Settings::AutoOpen::Music, "Music"},
          {Controllers::Settings::AutoOpen::Navigation, "Navigation"},
        }};

        lv_obj_t* cbOption[options.size()];
      };
    }
  }
}
