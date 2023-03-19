#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>

#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/CheckboxList.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingChimes : public Screen {
      public:
        SettingChimes(Pinetime::Controllers::Settings& settingsController);
        ~SettingChimes() override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        struct Option {
          Controllers::Settings::ChimesOption chimesOption;
          const char* name;
        };
        static constexpr std::array<Option, 4> options = {{
          {Controllers::Settings::ChimesOption::None, " Off"},
          {Controllers::Settings::ChimesOption::Hours, " Every hour"},
          {Controllers::Settings::ChimesOption::HalfHours, " Every 30 mins"},
          {Controllers::Settings::ChimesOption::LucidDream, " Lucid Dream Setting"}
        }};

        std::array<lv_obj_t*, options.size()> cbOption;

        Controllers::Settings& settingsController;
      };
    }
  }
}
