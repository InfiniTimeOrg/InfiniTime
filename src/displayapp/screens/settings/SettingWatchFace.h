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
      class SettingWatchFace : public Screen {
      public:
        SettingWatchFace(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingWatchFace() override;

        bool Refresh() override;

      private:
        CheckBoxes::Options options[4] = {
          {false, "Digital face"},
          {false, "Analog face"},
          {false, "PineTimeStyle"},
          {false, ""},
        };

        Controllers::Settings& settingsController;
        std::unique_ptr<Screen> screen;
        std::unique_ptr<Screen> CreateScreen();
      };
    }
  }
}
