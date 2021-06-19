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
      class SettingDisplay : public Screen {
      public:
        SettingDisplay(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingDisplay() override;

        bool Refresh() override;

      private:
        CheckBoxes::Options options[6] = {
          {false, "5s"},
          {false, "10s"},
          {false, "15s"},
          {false, "20s"},
          {false, "25s"},
          {false, "30s"},
        };

        Controllers::Settings& settingsController;
        std::unique_ptr<Screen> screen;
        std::unique_ptr<Screen> CreateScreen();
      };
    }
  }
}
