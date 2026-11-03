#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>

#include "displayapp/screens/Screen.h"
#include "displayapp/screens/ScreenList.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingBluetooth : public Screen {
      public:
        SettingBluetooth(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingBluetooth() override;

        bool OnTouchEvent(TouchEvents event) override;

      private:
        DisplayApp* app;
        Pinetime::Controllers::Settings& settings;

        ScreenList<2> screens;
        std::unique_ptr<Screen> screenBluetoothMain();
        std::unique_ptr<Screen> screenBluetoothSecurity();
      };
    }
  }
}
