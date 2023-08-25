#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>

#include "components/settings/Settings.h"
#include "displayapp/screens/ScreenList.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"
#include "displayapp/screens/CheckboxList.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingHeartRate : public Screen {
      public:
        SettingHeartRate(Pinetime::Applications::DisplayApp* app, Pinetime::Controllers::Settings& settings);
        ~SettingHeartRate() override;

        bool OnTouchEvent(TouchEvents event) override;

      private:
        DisplayApp* app;

        auto CreateScreenList() const;
        std::unique_ptr<Screen> CreateScreen(unsigned int screenNum) const;

        Pinetime::Controllers::Settings& settings;

        static constexpr const char* title = "Backg. Interval";
        static constexpr const char* symbol = Symbols::heartBeat;

        static constexpr int optionsPerScreen = 4;
        static constexpr int nScreens = 2;

        ScreenList<nScreens> screens;
      };
    }
  }
}
