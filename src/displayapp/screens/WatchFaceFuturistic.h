#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include <displayapp/screens/BatteryIcon.h>

namespace Pinetime {
  namespace Controllers {
    class Settings;
    class Battery;
    class Ble;
    class HeartRateController;
    class NotificationManager;
  }

  namespace Applications {
    namespace Screens {

      class WatchFaceFuturistic : public Screen {
      public:
        WatchFaceFuturistic(DisplayApp* app,
                            Controllers::DateTime& dateTimeController,
                            Controllers::Battery& batteryController,
                            Controllers::Ble& bleController,
                            Controllers::NotificationManager& notificationManager,
                            Controllers::HeartRateController& heartRateController);

        ~WatchFaceFuturistic() override;

        void Refresh() override;

      private:
        uint8_t sHour = 99;
        uint8_t sMinute = 99;
        uint8_t sSecond = 99;

        DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> currentDateTime;
        DirtyValue<uint8_t> heartbeat {};
        DirtyValue<bool> heartbeatRunning {};
        DirtyValue<bool> isCharging {};
        DirtyValue<uint8_t> batteryPercentRemaining {0};

        lv_style_t arc_fg_style;
        lv_style_t arc_bg_style;

        lv_obj_t* arcSec;
        lv_obj_t* arcMin;
        lv_obj_t* arcHour;
        lv_obj_t* timeDateLabel;
        lv_obj_t* heartbeatIcon;
        lv_obj_t* heartbeatValue;
        lv_obj_t* plugIcon;
        lv_obj_t* batteryStatus;
        lv_obj_t* batteryAlert;

        const Controllers::DateTime& dateTimeController;
        Controllers::Battery& batteryController;
        Controllers::Ble& bleController;
        Controllers::NotificationManager& notificationManager;
        Controllers::HeartRateController& heartRateController;

        lv_task_t* taskRefresh;
      };
    }
  }
}
