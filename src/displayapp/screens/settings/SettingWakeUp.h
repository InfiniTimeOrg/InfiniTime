#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/settings/Settings.h"
#include "displayapp/screens/Page.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/ScreenList.h"
#include "displayapp/widgets/PageIndicator.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingWakeUp : public Screen {
      public:
        SettingWakeUp(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingWakeUp() override;

        void UpdateSelected(lv_obj_t* object);

        bool OnTouchEvent(TouchEvents event) override;

      private:
        auto CreateScreenList();
        std::unique_ptr<Screen> CreateScreen(size_t screenNum);

        struct Option {
          Controllers::Settings::WakeUpMode wakeUpMode;
          const char* name;
        };

        static constexpr size_t numOptions = 6;
        static constexpr size_t optionsPerScreen = 4;
        static constexpr size_t nScreens = 2;

        Controllers::Settings& settingsController;

        ScreenList<nScreens> screens;

        static constexpr std::array<Option, numOptions> options = {{
          {Controllers::Settings::WakeUpMode::SingleTap, "Single Tap"},
          {Controllers::Settings::WakeUpMode::DoubleTap, "Double Tap"},
          {Controllers::Settings::WakeUpMode::RaiseWrist, "Raise Wrist"},
          {Controllers::Settings::WakeUpMode::Shake, "Shake Wake"},
          {Controllers::Settings::WakeUpMode::LowerWrist, "Lower Wrist"},
          {Controllers::Settings::WakeUpMode::ButtonUnlocks, "Button Unlock"},
        }};

        std::array<lv_obj_t*, numOptions> cbOption;
      };
    }
  }
}
