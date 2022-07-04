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

        void Increase();
        void Decrease();

      private:
        static constexpr std::array<uint16_t, 6> options = {5000, 7000, 10000, 15000, 20000, 30000};

        Controllers::Settings& settingsController;

        unsigned int setTimeout;

        lv_obj_t* timeoutLabel;
        lv_obj_t* increaseBtn;
        lv_obj_t* decreaseBtn;
      };
    }
  }
}
