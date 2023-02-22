#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/Colors.h"
#include "components/datetime/DateTimeController.h"
#include "components/ble/BleController.h"

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
      class WatchFacePineTimeStyle : public Screen {
      public:
        WatchFacePineTimeStyle(Controllers::DateTime& dateTimeController,
                               const Controllers::Battery& batteryController,
                               const Controllers::Ble& bleController,
                               Controllers::NotificationManager& notificationManager,
                               Controllers::Settings& settingsController,
                               Controllers::MotionController& motionController);
        ~WatchFacePineTimeStyle() override;

        bool OnTouchEvent(TouchEvents event) override;
        bool OnButtonPushed() override;

        void Refresh() override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        uint8_t displayedHour = -1;
        uint8_t displayedMinute = -1;
        uint8_t displayedSecond = -1;

        uint16_t currentYear = 1970;
        Controllers::DateTime::Months currentMonth = Pinetime::Controllers::DateTime::Months::Unknown;
        Controllers::DateTime::Days currentDayOfWeek = Pinetime::Controllers::DateTime::Days::Unknown;
        uint8_t currentDay = 0;
        uint32_t savedTick = 0;

        DirtyValue<uint8_t> batteryPercentRemaining {};
        DirtyValue<bool> isCharging {};
        DirtyValue<bool> bleState {};
        DirtyValue<bool> bleRadioEnabled {};
        DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> currentDateTime {};
        DirtyValue<bool> motionSensorOk {};
        DirtyValue<uint32_t> stepCount {};
        DirtyValue<bool> notificationState {};

        static Pinetime::Controllers::Settings::Colors GetNext(Controllers::Settings::Colors color);
        static Pinetime::Controllers::Settings::Colors GetPrevious(Controllers::Settings::Colors color);

        lv_obj_t* btnNextTime;
        lv_obj_t* btnPrevTime;
        lv_obj_t* btnNextBar;
        lv_obj_t* btnPrevBar;
        lv_obj_t* btnNextBG;
        lv_obj_t* btnPrevBG;
        lv_obj_t* btnReset;
        lv_obj_t* btnRandom;
        lv_obj_t* btnClose;
        lv_obj_t* btnSteps;
        lv_obj_t* timebar;
        lv_obj_t* sidebar;
        lv_obj_t* timeDD1;
        lv_obj_t* timeDD2;
        lv_obj_t* timeDD3;
        lv_obj_t* timeAMPM;
        lv_obj_t* dateDayOfWeek;
        lv_obj_t* dateDay;
        lv_obj_t* dateMonth;
        lv_obj_t* plugIcon;
        lv_obj_t* bleIcon;
        lv_obj_t* calendarOuter;
        lv_obj_t* calendarInner;
        lv_obj_t* calendarBar1;
        lv_obj_t* calendarBar2;
        lv_obj_t* calendarCrossBar1;
        lv_obj_t* calendarCrossBar2;
        lv_obj_t* notificationIcon;
        lv_obj_t* stepGauge;
        lv_obj_t* btnSetColor;
        lv_obj_t* btnSetOpts;
        lv_obj_t* lbl_btnSetColor;
        lv_obj_t* lbl_btnSetOpts;
        lv_obj_t* stepIcon;
        lv_obj_t* stepValue;
        lv_color_t needle_colors[1];

        BatteryIcon batteryIcon;

        Controllers::DateTime& dateTimeController;
        const Controllers::Battery& batteryController;
        const Controllers::Ble& bleController;
        Controllers::NotificationManager& notificationManager;
        Controllers::Settings& settingsController;
        Controllers::MotionController& motionController;

        void SetBatteryIcon();
        void CloseMenu();
        void AlignIcons();

        lv_task_t* taskRefresh;
      };
    }
  }
}
