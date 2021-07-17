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

        static bool UpdateArray(Pinetime::Applications::Screens::CheckBoxes::Options* options, uint8_t clicked);

      private:
        CheckBoxes::Options options[4] = {
          {false, "Single Tap"},
          {false, "Double Tap"},
          {false, "Raise Wrist"},
          {false, ""},
        };

        Controllers::Settings& settingsController;
        std::unique_ptr<Screens::CheckBoxes> screen;
        std::unique_ptr<Screens::CheckBoxes> CreateScreen();
      };
    }
  }
}
