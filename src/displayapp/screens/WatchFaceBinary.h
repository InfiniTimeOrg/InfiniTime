#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include "Screen.h"
#include "ScreenList.h"
#include "components/datetime/DateTimeController.h"

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
        WatchFaceBinary(DisplayApp* app,
                         Controllers::DateTime& dateTimeController,
                         Controllers::Battery& batteryController,
                         Controllers::Ble& bleController,
                         Controllers::NotificationManager& notificatioManager,
                         Controllers::Settings& settingsController,
                         Controllers::HeartRateController& heartRateController,
                         Controllers::MotionController& motionController);
        ~WatchFaceBinary() override;

        bool Refresh() override;

        void OnObjectEvent(lv_obj_t* pObj, lv_event_t i);

      private:
        uint8_t minutes_old = 0;

        uint16_t currentYear = 1970;
        Pinetime::Controllers::DateTime::Months currentMonth = Pinetime::Controllers::DateTime::Months::Unknown;
        Pinetime::Controllers::DateTime::Days currentDayOfWeek = Pinetime::Controllers::DateTime::Days::Unknown;
        uint8_t currentDay = 0;

        DirtyValue<uint8_t> batteryPercentRemaining {};
        DirtyValue<uint16_t> batteryVoltage {};
        DirtyValue<bool> bleState {};
        DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> currentDateTime {};
        DirtyValue<bool> motionSensorOk {};
        DirtyValue<uint32_t> stepCount {};
        DirtyValue<uint8_t> heartbeat {};
        DirtyValue<bool> heartbeatRunning {};
        DirtyValue<bool> notificationState {};

        lv_obj_t* label_year;
        lv_obj_t* label_day;
        lv_obj_t* backgroundLabel;
        lv_obj_t* batteryIcon;
        lv_obj_t* bleIcon;
        lv_obj_t* batteryPlug;
        lv_obj_t* heartbeatIcon;
        lv_obj_t* heartbeatValue;
        lv_obj_t* heartbeatBpm;
        lv_obj_t* stepIcon;
        lv_obj_t* stepValue;
        lv_obj_t* notificationIcon;

        // Binary Watch
        // Time
        static constexpr uint8_t LED_SIZE1 = 37;
        static constexpr uint8_t LED_RING1 = 2;
        static constexpr uint8_t LED_SPACE_H1 = (LED_SIZE1/12);
        static constexpr uint8_t LED_SPACE_V1 = (LED_SIZE1/4);
        #define LED_COL_MIN_ON   lv_color_hex(0xDD0000)
        #define LED_COL_MIN_OFF  lv_color_hex(0x220000)
        #define LED_COL_HOUR_ON  lv_color_hex(0x00DD00)
        #define LED_COL_HOUR_OFF lv_color_hex(0x002200)
        #define LED_COL_RING     lv_color_hex(0x999999)
        lv_obj_t * minLED0;
        lv_obj_t * minLED1;
        lv_obj_t * minLED2;
        lv_obj_t * minLED3;
        lv_obj_t * minLED4;
        lv_obj_t * minLED5;

        lv_obj_t * hourLED0;
        lv_obj_t * hourLED1;
        lv_obj_t * hourLED2;
        lv_obj_t * hourLED3;
        lv_obj_t * hourLED4;
        
        // Date
        static constexpr uint8_t LED_SIZE2 = 18;
        static constexpr uint8_t LED_RING2 = 2;
        static constexpr uint8_t LED_SPACE_H2 = (LED_SIZE2/6);
        static constexpr uint8_t LED_SPACE_V2 = (LED_SIZE2/4);
        #define LED_COL_DAY_ON	lv_color_hex(0xCCCCCC)
        #define LED_COL_DAY_OFF lv_color_hex(0x222222)  
        #define LED_COL_MON_ON  lv_color_hex(0xCCCCCC)
        #define LED_COL_MON_OFF lv_color_hex(0x222222)  
        #define LED_COL_RING    lv_color_hex(0x999999)    
        lv_obj_t * dayLED0;
        lv_obj_t * dayLED1;
        lv_obj_t * dayLED2;
        lv_obj_t * dayLED3;
        lv_obj_t * dayLED4;

        lv_obj_t * monLED0;
        lv_obj_t * monLED1;
        lv_obj_t * monLED2;
        lv_obj_t * monLED3;

        Controllers::DateTime& dateTimeController;
        Controllers::Battery& batteryController;
        Controllers::Ble& bleController;
        Controllers::NotificationManager& notificatioManager;
        Controllers::Settings& settingsController;
        Controllers::HeartRateController& heartRateController;
        Controllers::MotionController& motionController;
      };
    }
  }
}
