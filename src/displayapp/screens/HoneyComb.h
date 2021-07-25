#pragma once

#include <lvgl/lvgl.h>
#include <cstdint>
#include <memory>
#include "Screen.h"
#include "../Apps.h"
#include "components/datetime/DateTimeController.h"
#include "components/settings/Settings.h"
#include "components/datetime/DateTimeController.h"
#include "components/battery/BatteryController.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class HoneyComb : public Screen {
      public:
        struct Applications {
          const char* icon;
          Pinetime::Applications::Apps application;
        };

        explicit HoneyComb(uint8_t screenID,
                      uint8_t numScreens,
                      DisplayApp* app,
                      Controllers::Settings& settingsController,
                      Pinetime::Controllers::Battery& batteryController,
                      Controllers::DateTime& dateTimeController,
                      std::array<Applications, 8>& applications);

        ~HoneyComb() override;

        bool Refresh() override;
        void UpdateScreen();
        void OnObjectEvent(lv_obj_t* obj, lv_event_t event);

      private:
        Pinetime::Controllers::Battery& batteryController;
        Controllers::DateTime& dateTimeController;

        lv_task_t* taskUpdate;

        lv_obj_t* labelTime;
        lv_obj_t* batteryIcon;
        lv_point_t pageIndicatorBasePoints[2];
        lv_point_t pageIndicatorPoints[2];
        lv_obj_t* pageIndicatorBase;
        lv_obj_t* pageIndicator;
        lv_obj_t* buttons[8];

        Pinetime::Applications::Apps apps[8];
      };
    }
  }
}
