#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingSteps : public Screen {
      public:
        SettingSteps(Pinetime::Controllers::Settings& settingsController);
        ~SettingSteps() override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        Controllers::Settings& settingsController;

        lv_obj_t* stepValue;
        lv_obj_t* btnPlus;
        lv_obj_t* btnMinus;
      };
    }
  }
}
