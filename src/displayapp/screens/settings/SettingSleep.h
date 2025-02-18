#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingSleep : public Screen {
      public:
        SettingSleep(Pinetime::Controllers::Settings& settingsController);
        ~SettingSleep() override;

        void UpdateSelected(lv_obj_t* object);

      private:
        struct Option {
          Controllers::Settings::SleepOption sleepOption;
          const char* name;
        };

        Controllers::Settings& settingsController;
        static constexpr std::array<Option, 4> options = {{
          {Controllers::Settings::SleepOption::AllowChimes, "Allow Chimes"},
          {Controllers::Settings::SleepOption::AllowNotify, "Allow Notify"},
          {Controllers::Settings::SleepOption::EnableAOD, "Enable AOD"},
          {Controllers::Settings::SleepOption::DisableBle, "Disable BLE"},
        }};

        lv_obj_t* cbOption[options.size()];
      };
    }
  }
}
