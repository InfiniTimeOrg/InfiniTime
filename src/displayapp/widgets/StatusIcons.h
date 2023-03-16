#pragma once

#include <lvgl/lvgl.h>

#include "displayapp/screens/Screen.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "displayapp/screens/BatteryIcon.h"
#include "utility/DirtyValue.h"

namespace Pinetime {
  namespace Applications {
    namespace Widgets {
      class StatusIcons {
      public:
        StatusIcons(const Controllers::Battery& batteryController, const Controllers::Ble& bleController);
        void Align();
        void Create();

        lv_obj_t* GetObject() {
          return container;
        }

        void Update();

      private:
        Screens::BatteryIcon batteryIcon;
        const Pinetime::Controllers::Battery& batteryController;
        const Controllers::Ble& bleController;

        Utility::DirtyValue<uint8_t> batteryPercentRemaining {};
        Utility::DirtyValue<bool> powerPresent {};
        Utility::DirtyValue<bool> bleState {};
        Utility::DirtyValue<bool> bleRadioEnabled {};

        lv_obj_t* bleIcon;
        lv_obj_t* batteryPlug;
        lv_obj_t* container;
      };
    }
  }
}
