#pragma once

#include <cstdint>
#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>
#include "components/datetime/DateTimeController.h"
#include "components/brightness/BrightnessController.h"
#include "components/motor/MotorController.h"
#include "components/settings/Settings.h"
#include "components/battery/BatteryController.h"
#include "displayapp/widgets/StatusIcons.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class QuickSettings : public Screen {
      public:
        QuickSettings(DisplayApp* app,
                      const Pinetime::Controllers::Battery& batteryController,
                      Controllers::DateTime& dateTimeController,
                      Controllers::BrightnessController& brightness,
                      Controllers::MotorController& motorController,
                      Pinetime::Controllers::Settings& settingsController,
                      const Controllers::Ble& bleController);

        ~QuickSettings() override;

        void OnButtonEvent(lv_obj_t* object);

        void UpdateScreen();

      private:
        DisplayApp* app;
        Controllers::DateTime& dateTimeController;
        Controllers::BrightnessController& brightness;
        Controllers::MotorController& motorController;
        Controllers::Settings& settingsController;

        lv_task_t* taskUpdate;
        lv_obj_t* label_time;

        lv_style_t btn_style;

        lv_obj_t* btn1;
        lv_obj_t* btn1_lvl;
        lv_obj_t* btn2;
        lv_obj_t* btn3;
        lv_obj_t* btn3_lvl;
        lv_obj_t* btn4;

        Widgets::StatusIcons statusIcons;
      };
    }
  }
}
