#pragma once

#include <array>
#include <cstdint>
#include <lvgl/lvgl.h>

#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/CheckboxList.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingBluetoothSecurity : public Screen {
      public:
        SettingBluetoothSecurity(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~SettingBluetoothSecurity() override;

        void ToggleSecurity();
        void TogglePairing();

      private:
        DisplayApp* app;
        Pinetime::Controllers::Settings& settings;

        bool secured;
        void UpdateRebootMessage();

        lv_obj_t* securityCheckbox;
        lv_obj_t* pairingCheckbox;
        lv_obj_t* statusLabel;
      };
    }
  }
}
