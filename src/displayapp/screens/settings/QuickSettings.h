#pragma once

#include <cstdint>
#include <FreeRTOS.h>
#include <timers.h>
#include "displayapp/screens/Screen.h"
#include <lvgl/lvgl.h>
#include "components/datetime/DateTimeController.h"
#include "components/brightness/BrightnessController.h"
#include "components/motor/MotorController.h"
#include "components/settings/Settings.h"
#include "components/battery/BatteryController.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class QuickSettings : public Screen {
      public:
        QuickSettings(DisplayApp* app,
                      Controllers::BrightnessController& brightness,
                      Controllers::MotorController& motorController,
                      Pinetime::Controllers::Settings& settingsController);

        ~QuickSettings() override;

        void OnButtonEvent(lv_obj_t* object, lv_event_t event);

        void UpdateScreen();

      private:
        Controllers::BrightnessController& brightness;
        Controllers::MotorController& motorController;
        Controllers::Settings& settingsController;

        lv_task_t* taskUpdate;
        DisplayApp::StatusBar statusBar;

        lv_style_t btn_style;

        lv_obj_t* btn1;
        lv_obj_t* btn1_lvl;
        lv_obj_t* btn2;
        lv_obj_t* btn3;
        lv_obj_t* btn3_lvl;
        lv_obj_t* btn4;
      };
    }
  }
}
