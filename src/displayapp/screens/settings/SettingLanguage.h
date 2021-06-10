#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include <cstring>

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingLanguage : public Screen {
      public:
        SettingLanguage(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingLanguage() override;

        bool Refresh() override;
        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        Controllers::Settings& settingsController;
      };
    }
  }
}
