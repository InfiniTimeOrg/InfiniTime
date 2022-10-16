#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>

#include "displayapp/screens/ScreenList.h"
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/Symbols.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingWatchFace : public Screen {
      public:
        SettingWatchFace(DisplayApp* app, Pinetime::Controllers::Settings& settingsController, Pinetime::Controllers::FS& filesystem);
        ~SettingWatchFace() override;

        bool OnTouchEvent(TouchEvents event) override;

      private:
        Controllers::Settings& settingsController;
        Pinetime::Controllers::FS& filesystem;
        ScreenList<2> screens;

        static constexpr const char* title = "Watch face";
        static constexpr const char* symbol = Symbols::home;
        std::unique_ptr<Screen> CreateScreen1();
        std::unique_ptr<Screen> CreateScreen2();
      };
    }
  }
}
