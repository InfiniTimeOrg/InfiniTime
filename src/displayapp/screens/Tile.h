#pragma once

#include <lvgl/lvgl.h>
#include <cstdint>
#include <memory>
#include "displayapp/screens/Screen.h"
#include "displayapp/apps/Apps.h"
#include "components/datetime/DateTimeController.h"
#include "components/settings/Settings.h"
#include "components/battery/BatteryController.h"
#include "displayapp/widgets/PageIndicator.h"
#include "displayapp/widgets/StatusIcons.h"
#include "utility/DirtyValue.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Tile : public Screen {
      public:
        struct Applications {
          const char* icon;
          Pinetime::Applications::Apps application;
          bool enabled;
        };

        explicit Tile(uint8_t screenID,
                      uint8_t numScreens,
                      DisplayApp* app,
                      Controllers::Settings& settingsController,
                      const Controllers::Battery& batteryController,
                      const Controllers::Ble& bleController,
                      const Controllers::AlarmController& alarmController,
                      Controllers::DateTime& dateTimeController,
                      std::array<Applications, 6>& applications);

        ~Tile() override;

        void Refresh() override;
        void OnValueChangedEvent(lv_obj_t* obj, uint32_t buttonId);

      private:
        DisplayApp* app;
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>> currentDateTime {};
        Controllers::DateTime& dateTimeController;

        lv_task_t* taskRefresh;

        lv_obj_t* label_time;
        lv_obj_t* btnm1;

        Widgets::PageIndicator pageIndicator;
        Widgets::StatusIcons statusIcons;

        const char* btnmMap[8];
        Pinetime::Applications::Apps apps[6];
      };
    }
  }
}
