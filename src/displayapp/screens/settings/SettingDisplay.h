#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>

#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingDisplay : public Screen {
      public:
        SettingDisplay(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingDisplay() override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        static constexpr std::array<uint16_t, 4> options = {5000, 15000, 20000, 30000};

        Controllers::Settings& settingsController;
        lv_obj_t* cbOption[options.size()];
      };
    }
  }
}
