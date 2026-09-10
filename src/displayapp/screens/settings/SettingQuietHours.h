#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingQuietHours : public Screen {
      public:
        SettingQuietHours(Pinetime::Controllers::Settings& settingsController);
        ~SettingQuietHours() override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);
        void ToggleEnabled();

      private:
        Controllers::Settings& settingsController;

        lv_obj_t* enabledCheckbox;
        lv_obj_t* startValue;
        lv_obj_t* endValue;
        lv_obj_t* btnStartPlus;
        lv_obj_t* btnStartMinus;
        lv_obj_t* btnEndPlus;
        lv_obj_t* btnEndMinus;
      };
    }
  }
}
