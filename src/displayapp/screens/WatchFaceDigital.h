#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "components/ble/BleController.h"
#include "displayapp/widgets/StatusIcons.h"

namespace Pinetime {
  namespace Controllers {
    class Settings;
    class Battery;
    class Ble;
    class NotificationManager;
    class HeartRateController;
    class MotionController;
  }

  namespace Applications {
    namespace Screens {

      class WatchFaceDigital : public Screen {
      public:
        WatchFaceDigital(DisplayApp* app,
                         Controllers::DateTime& dateTimeController,
                         Controllers::Battery& batteryController,
                         Controllers::Ble& bleController,
                         Controllers::NotificationManager& notificatioManager,
                         Controllers::Settings& settingsController,
                         Controllers::HeartRateController& heartRateController,
                         Controllers::MotionController& motionController);
        ~WatchFaceDigital() override;

        void Refresh() override;

      private:
        uint8_t displayedHour = -1;
        uint8_t displayedMinute = -1;

        uint16_t currentYear = 1970;
        Controllers::DateTime::Months currentMonth = Pinetime::Controllers::DateTime::Months::Unknown;
        Controllers::DateTime::Days currentDayOfWeek = Pinetime::Controllers::DateTime::Days::Unknown;
        uint8_t currentDay = 0;

        DirtyValue<uint8_t> batteryPercentRemaining {};
        DirtyValue<bool> powerPresent {};
        DirtyValue<bool> bleState {};
        DirtyValue<bool> bleRadioEnabled {};
        DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> currentDateTime {};
        DirtyValue<bool> motionSensorOk {};
        // DirtyValue<uint32_t> stepCount {};
        // DirtyValue<uint8_t> heartbeat {};
        // DirtyValue<bool> heartbeatRunning {};
        DirtyValue<bool> notificationState {};
        DirtyValue<int32_t> temperature {};

        lv_obj_t* label_time;
        lv_obj_t* label_time_ampm;
        lv_obj_t* label_date;
        // lv_obj_t* heartbeatIcon;
        // lv_obj_t* heartbeatValue;
        // lv_obj_t* stepIcon;
        // lv_obj_t* stepValue;
        lv_obj_t* notificationIcon;
        lv_obj_t* temperatureValue;

        Controllers::DateTime& dateTimeController;
        Controllers::NotificationManager& notificatioManager;
        Controllers::Settings& settingsController;
        Controllers::HeartRateController& heartRateController;
        Controllers::MotionController& motionController;

        lv_task_t* taskRefresh;
        Widgets::StatusIcons statusIcons;
      };
    }
  }
}
