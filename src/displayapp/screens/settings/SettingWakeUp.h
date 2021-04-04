#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingWakeUp : public Screen{
        public:
          SettingWakeUp(DisplayApp* app, Pinetime::Controllers::Settings &settingsController);
          ~SettingWakeUp() override;

          bool Refresh() override;
          void UpdateSelected(lv_obj_t *object, lv_event_t event);
         
        private:          

          Controllers::Settings& settingsController;
          uint8_t optionsTotal;
          lv_obj_t * cbOption[3];
          
      };
    }
  }
}
