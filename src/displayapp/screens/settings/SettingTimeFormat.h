#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/ScreenList.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class SettingTimeFormat : public Screen {
      public:
        SettingTimeFormat(DisplayApp* app,
                           Pinetime::Controllers::Settings& settingsController);
        ~SettingTimeFormat() override;

        bool OnTouchEvent(TouchEvents event) override;

      private:
        Controllers::Settings& settingsController;

        ScreenList<2> screens;
        std::unique_ptr<Screen> screenDateFormat();
        std::unique_ptr<Screen> screenClockFormat();
      };
    }
  }
}
