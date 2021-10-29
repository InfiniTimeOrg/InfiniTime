#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingIncomingCalls : public Screen {
      public:
        SettingIncomingCalls(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingIncomingCalls() override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        Controllers::Settings& settingsController;
        uint8_t optionsTotal;
        lv_obj_t* cbOption[2];
      };

      class NotificationItem : public Screen {
      public:
        NotificationItem(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);

      private:
        Controllers::Settings& settingsController;
        uint8_t optionsTotal;
        lv_obj_t* cbOption[2];
      };
    }
  }
}
