#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include <array>

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingChimes : public Screen {
      public:
        SettingChimes(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingChimes() override;

        void UpdateFrequency(lv_obj_t* object, lv_event_t event);

      private:
        struct Option {
          uint8_t value;
          const char* label;
        };

        static constexpr std::array<Option, 4> frequencyOptions = {{
          {0, " Off"},
          {60, " Every hour"},
          {30, " Every 30 mins"},
          {15, " Every 15 mins"}
        }};

        std::array<lv_obj_t*, frequencyOptions.size()> frequencyCheckBoxes;

        Controllers::Settings& settingsController;
      };
    }
  }
}
