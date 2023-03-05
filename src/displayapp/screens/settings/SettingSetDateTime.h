#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/ScreenList.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class SettingSetDateTime : public Screen {
      public:
        SettingSetDateTime(DisplayApp* app,
                           Pinetime::Controllers::DateTime& dateTimeController,
                           Pinetime::Controllers::Settings& settingsController);
        ~SettingSetDateTime() override;

        bool OnTouchEvent(TouchEvents event) override;
        void Advance();
        void Quit();

      private:
        DisplayApp* app;
        Controllers::DateTime& dateTimeController;
        Controllers::Settings& settingsController;

        ScreenList<2> screens;
        std::unique_ptr<Screen> screenSetDate();
        std::unique_ptr<Screen> screenSetTime();
      };
    }
  }
}
