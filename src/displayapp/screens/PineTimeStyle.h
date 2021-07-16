#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <cstdint>
#include "WatchFaceBase.h"
#include "components/datetime/DateTime.h"

namespace Pinetime {
  namespace Controllers {
    class DateTimeController;
    class Settings;
    class Battery;
    class Ble;
    class NotificationManager;
    class HeartRateController;
    class MotionController;
  }

  namespace Applications {
    namespace Screens {
      class PineTimeStyle : public WatchFaceBase {
      public:
        PineTimeStyle(DisplayApp* app,
                      Controllers::DateTimeController const& dateTimeController,
                      Controllers::Battery const& batteryController,
                      Controllers::Ble const& bleController,
                      Controllers::NotificationManager const& notificationManager,
                      Controllers::Settings& settingsController,
                      Controllers::HeartRateController const& heartRateController,
                      Controllers::MotionController const& motionController);
        ~PineTimeStyle() override;

        bool Refresh() override;

        void OnObjectEvent(lv_obj_t* pObj, lv_event_t i);

      private:
        DirtyValue<uint8_t> hour{};
        DirtyValue<uint8_t> minute{};

        lv_obj_t* timebar;
        lv_obj_t* sidebar;
        lv_obj_t* timeDD1;
        lv_obj_t* timeDD2;
        lv_obj_t* timeAMPM;
        lv_obj_t* dateDayOfWeek;
        lv_obj_t* dateDay;
        lv_obj_t* dateMonth;
        lv_obj_t* backgroundLabel;
        lv_obj_t* batteryIcon;
        lv_obj_t* bleIcon;
        lv_obj_t* batteryPlug;
        lv_obj_t* calendarOuter;
        lv_obj_t* calendarInner;
        lv_obj_t* calendarBar1;
        lv_obj_t* calendarBar2;
        lv_obj_t* calendarCrossBar1;
        lv_obj_t* calendarCrossBar2;
        lv_obj_t* heartbeatIcon;
        lv_obj_t* heartbeatValue;
        lv_obj_t* heartbeatBpm;
        lv_obj_t* notificationIcon;
        lv_obj_t* stepGauge;
        lv_color_t needle_colors[1];
      };
    }
  }
}
