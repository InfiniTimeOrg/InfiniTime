#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>

#include "displayapp/screens/Screen.h"
#include "displayapp/screens/ScreenList.h"
#include "displayapp/screens/Symbols.h"
#include "components/settings/Settings.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingWakeUp : public Screen {
      public:
        SettingWakeUp(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingWakeUp() override;

        bool OnTouchEvent(TouchEvents event) override;

      private:
        auto CreateScreenList() const;
        std::unique_ptr<Screen> CreateScreen(unsigned int screenNum) const;

        struct Option {
          Controllers::Settings::WakeUpMode wakeUpMode;
          const char* name;
        };

        static constexpr int settingsPerScreen = 4;
        static constexpr int optionsCount = 5;
        static constexpr int nScreens = (optionsCount - 1) / settingsPerScreen + 1;

        Controllers::Settings& settingsController;
        static constexpr std::array<Option, optionsCount> options = {{
          {Controllers::Settings::WakeUpMode::SingleTap, "Single Tap"},
          {Controllers::Settings::WakeUpMode::DoubleTap, "Double Tap"},
          {Controllers::Settings::WakeUpMode::RaiseWrist, "Raise Wrist"},
          {Controllers::Settings::WakeUpMode::Shake, "Shake Wake"},
          {Controllers::Settings::WakeUpMode::LowerWrist, "Lower Wrist"},
        }};

        static constexpr const char* title = "Wake Up";
        static constexpr const char* symbol = Symbols::eye;

        ScreenList<nScreens> screens;
      };
    }
  }
}
