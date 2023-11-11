#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "components/ble/BleController.h"
#include "displayapp/widgets/StatusIcons.h"
#include "utility/DirtyValue.h"

// #define LV_COLOR_MATRIX_GREEN LV_COLOR_MAKE(0x00, 0xC0, 0x16)
#define BINARY_ON_COLOR        LV_COLOR_RED
#define BINARY_OFF_COLOR       LV_COLOR_GRAY
#define BINARY_SECONDS_ENABLED false

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

      class WatchFaceBinary : public Screen {
      public:
        WatchFaceBinary(Controllers::DateTime& dateTimeController,
                        const Controllers::Battery& batteryController,
                        const Controllers::Ble& bleController,
                        Controllers::NotificationManager& notificationManager,
                        Controllers::Settings& settingsController,
                        Controllers::HeartRateController& heartRateController,
                        Controllers::MotionController& motionController);
        ~WatchFaceBinary() override;

        void Refresh() override;

      private:
        uint8_t displayedHours = -1;
        uint8_t displayedMinutes = -1;
        bool is12HourModeSet = false;
        Controllers::Settings::ClockType lastClockType;

        Utility::DirtyValue<uint8_t> batteryPercentRemaining {};
        Utility::DirtyValue<bool> powerPresent {};
        Utility::DirtyValue<bool> bleState {};
        Utility::DirtyValue<bool> bleRadioEnabled {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>> currentDateTime {};
        Utility::DirtyValue<uint32_t> stepCount {};
        Utility::DirtyValue<uint8_t> heartbeat {};
        Utility::DirtyValue<bool> heartbeatRunning {};
        Utility::DirtyValue<bool> notificationState {};
        using days = std::chrono::duration<int32_t, std::ratio<86400>>; // TODO: days is standard in c++20
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, days>> currentDate;

        lv_obj_t* heartbeatIcon;
        lv_obj_t* heartbeatValue;
        lv_obj_t* stepIcon;
        lv_obj_t* stepValue;
        lv_obj_t* notificationIcon;

        lv_obj_t* lableDate;
        lv_obj_t* labelAM;
        lv_obj_t* labelPM;
        lv_obj_t* hourPoints[5];
        lv_obj_t* minutePoints[6];
#if BINARY_SECONDS_ENABLED
        lv_obj_t* secondPoints[6];
#endif

        Controllers::DateTime& dateTimeController;
        Controllers::NotificationManager& notificationManager;
        Controllers::Settings& settingsController;
        Controllers::HeartRateController& heartRateController;
        Controllers::MotionController& motionController;

        lv_task_t* taskRefresh;
        Widgets::StatusIcons statusIcons;
      };
    }
  }
}
