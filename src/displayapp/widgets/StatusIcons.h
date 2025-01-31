#pragma once

#include <lvgl/lvgl.h>

#include "displayapp/screens/Screen.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/alarm/AlarmController.h"
#include "displayapp/screens/BatteryIcon.h"
#include "utility/DirtyValue.h"

namespace Pinetime {
  namespace Applications {
    namespace Widgets {
      class StatusIcons {
      public:
        StatusIcons(const Controllers::Battery& batteryController,
                    const Controllers::Ble& bleController,
                    const Controllers::AlarmController& alarmController,
                    const Controllers::Timer& timer);
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
        const Controllers::AlarmController& alarmController;
        const Controllers::Timer& timer;

        Utility::DirtyValue<uint8_t> batteryPercentRemaining {};
        Utility::DirtyValue<bool> powerPresent {};
        Utility::DirtyValue<bool> bleState {};
        Utility::DirtyValue<bool> bleRadioEnabled {};
        Utility::DirtyValue<bool> alarmEnabled {};

        lv_obj_t* container;
        lv_obj_t* timerContainer;
        lv_obj_t* timerIcon;
        lv_obj_t* timeRemaining;
        lv_obj_t* bleIcon;
        lv_obj_t* batteryPlug;
        lv_obj_t* soloTimerIcon;
        lv_obj_t* alarmIcon;
      };
    }
  }
}
