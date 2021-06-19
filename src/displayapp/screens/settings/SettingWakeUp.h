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
      class SettingWakeUp : public Screen {
      public:
        SettingWakeUp(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingWakeUp() override;

        bool Refresh() override;

      private:
        CheckBoxes::Options options[5] = {
          {false, "None"},
          {false, "Single Tap"},
          {false, "Double Tap"},
          {false, "Raise Wrist"},
          {false, ""},
        };

        Controllers::Settings& settingsController;
        std::unique_ptr<Screen> screen;
        std::unique_ptr<Screen> CreateScreen();
      };
    }
  }
}
