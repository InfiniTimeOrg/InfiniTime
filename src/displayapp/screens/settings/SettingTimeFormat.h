#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include <memory>
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/CheckBoxes.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class SettingTimeFormat : public Screen {
      public:
        SettingTimeFormat(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingTimeFormat() override;

        bool Refresh() override;

      private:
        CheckBoxes::Options options[3] = {
          {false, "12-hour"},
          {false, "24-hour"},
          {false, ""},
        };

        Controllers::Settings& settingsController;
        std::unique_ptr<Screen> screen;
        std::unique_ptr<Screen> CreateScreen();
      };
    }
  }
}
