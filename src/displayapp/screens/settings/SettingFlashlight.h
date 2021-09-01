#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include <displayapp/Colors.h>

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingFlashlight : public Screen {
      public:
        SettingFlashlight(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingFlashlight() override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        Controllers::Settings& settingsController;
        uint8_t optionsTotal;
        lv_obj_t* cbOption[4];
      };
    }
  }
}
