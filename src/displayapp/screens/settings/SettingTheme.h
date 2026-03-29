#pragma once

#include <lvgl/lvgl.h>

#include "displayapp/screens/Screen.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingTheme : public Screen {
      public:
        SettingTheme(Pinetime::Applications::DisplayApp* app);
        ~SettingTheme() override;
        void OnReloadButtonClicked();

      private:
        Pinetime::Applications::DisplayApp* app;
        lv_obj_t* statusLabel;
        lv_obj_t* reloadButton;

        void UpdateThemeStatus();

        static void ReloadButtonCallback(lv_obj_t* obj, lv_event_t event);
      };
    }
  }
}
