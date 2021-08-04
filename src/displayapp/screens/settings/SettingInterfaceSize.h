#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingInterfaceSize : public Screen {
      public:
        SettingInterfaceSize(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingInterfaceSize() override;

        bool Refresh() override;
        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        Controllers::Settings& settingsController;
        static constexpr uint8_t optionsTotal = 2;
        lv_obj_t* cbOption[optionsTotal];
      };
    }
  }
}
