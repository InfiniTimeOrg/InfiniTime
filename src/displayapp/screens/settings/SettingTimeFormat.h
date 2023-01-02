#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>

#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/settings/SettingScreenBase.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingTimeFormat : public Screen {
      public:
        SettingTimeFormat(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingTimeFormat() override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        Controllers::Settings& settingsController;

        struct Option {
          Controllers::Settings::ClockType clockType;
          const char* name;
        };

        static constexpr size_t nEntries = 2;

        static constexpr std::array<Option, nEntries> options = {{
          {Controllers::Settings::ClockType::H12, "12-hour"},
          {Controllers::Settings::ClockType::H24, "24-hour"},
        }};

        Pinetime::Applications::Screens::SettingScreenBase::Entry entries[nEntries] = {
          {options[0].name, settingsController.GetClockType() == Controllers::Settings::ClockType::H12},
          {options[1].name, settingsController.GetClockType() == Controllers::Settings::ClockType::H24},
        };
        lv_obj_t* objectArray[nEntries];
        SettingScreenBase settingScreen;
      };
    }
  }
}
