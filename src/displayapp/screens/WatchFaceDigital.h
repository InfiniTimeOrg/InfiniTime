#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <cstdint>
#include "WatchFaceBase.h"
#include "components/datetime/DateTime.h"

namespace Pinetime {
  namespace Controllers {
    class Settings;
    class Battery;
    class Ble;
    class NotificationManager;
    class HeartRateController;
    class MotionController;
    class DateTimeController;
  }

  namespace Applications {
    class DisplayApp;

    namespace Screens {

      class WatchFaceDigital : public WatchFaceBase {
      public:
        WatchFaceDigital(DisplayApp* app,
                         Controllers::DateTimeController const& dateTimeController,
                         Controllers::Battery const& batteryController,
                         Controllers::Ble const& bleController,
                         Controllers::NotificationManager const& notificationManager,
                         Controllers::Settings& settingsController,
                         Controllers::HeartRateController const& heartRateController,
                         Controllers::MotionController const& motionController);
        ~WatchFaceDigital() override;

        bool Refresh() override;

        void OnObjectEvent(lv_obj_t* pObj, lv_event_t i);

      private:
        DirtyValue<uint8_t> hour{};
        DirtyValue<uint8_t> minute{};

        lv_obj_t* label_time;
        lv_obj_t* label_time_ampm;
        lv_obj_t* label_date;
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
      };
    }
  }
}
