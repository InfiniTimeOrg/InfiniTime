#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingQuietHour : public Screen {
      public:
        SettingQuietHour(Pinetime::Controllers::Settings& settingsController);
        ~SettingQuietHour() override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);
        void UpdateButton();

      private:
        lv_obj_t *cbOption[2], *colon[2], *btnTime[2][2], *txtTime[2][2], *time_container[2];

        Controllers::Settings& settingsController;
      };
    }
  }
}
