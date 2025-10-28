#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class SettingPomodoro : public Screen {
      public:
        SettingPomodoro(Pinetime::Controllers::Settings& settingsController);
        ~SettingPomodoro() override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);
        bool OnTouchEvent(Pinetime::Applications::TouchEvents event) override;

      private:
        Controllers::Settings& settingsController;

        lv_obj_t* title;
        lv_obj_t* settingValue;
        lv_obj_t* btnPlus;
        lv_obj_t* btnMinus;
        
        uint8_t currentSetting;

        void UpdateDisplay();
      };
    }
  }
}