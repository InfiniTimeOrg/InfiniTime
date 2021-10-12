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
                      Pinetime::Controllers::Battery& batteryController,
                      Controllers::DateTime& dateTimeController,
                      Controllers::BrightnessController& brightness,
                      Controllers::MotorController& motorController,
                      Pinetime::Controllers::Settings& settingsController);

        ~QuickSettings() override;

        void OnButtonEvent(lv_obj_t* object, lv_event_t* event);

        void UpdateScreen();

      private:
        Pinetime::Controllers::Battery& batteryController;
        Controllers::DateTime& dateTimeController;
        Controllers::BrightnessController& brightness;
        Controllers::MotorController& motorController;
        Controllers::Settings& settingsController;

        lv_timer_t* taskUpdate;
        lv_obj_t* batteryIcon;
        lv_obj_t* label_time;

        lv_style_t btn_style;

        lv_obj_t* brightnessButton;
        lv_obj_t* brightnessLabel;
        lv_obj_t* flashlightButton;
        lv_obj_t* flashlightLabel;
        lv_obj_t* notificationButton;
        lv_obj_t* notificationLabel;
        lv_obj_t* settingsButton;
        lv_obj_t* settingsLabel;
        
        lv_obj_t* container1;
      };
    }
  }
}
