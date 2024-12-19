#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include <displayapp/Controllers.h>
#include "displayapp/screens/Screen.h"
#include "displayapp/widgets/StatusIcons.h"
#include "components/datetime/DateTimeController.h"
#include "components/ble/BleController.h"
#include "utility/DirtyValue.h"

namespace Pinetime {
  namespace Controllers {
    class Settings;
    class Battery;
    class Ble;
    class NotificationManager;
    class MotionController;
  }

  namespace Applications {
    namespace Screens {

      class WatchFaceTrans : public Screen {
      public:
        WatchFaceTrans(Controllers::DateTime& dateTimeController,
                       const Controllers::Battery& batteryController,
                       const Controllers::Ble& bleController,
                       Controllers::NotificationManager& notificationManager,
                       Controllers::Settings& settingsController,
                       Controllers::MotionController& motionController);
        ~WatchFaceTrans() override;

        void Refresh() override;

      private:
        Utility::DirtyValue<int> batteryPercentRemaining {};
        Utility::DirtyValue<bool> powerPresent {};
        Utility::DirtyValue<bool> bleState {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>> currentDateTime {};
        Utility::DirtyValue<uint32_t> stepCount {};
        Utility::DirtyValue<bool> notificationState {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::days>> currentDate;

        lv_obj_t* topBlueBackground;
        lv_obj_t* topPinkBackground;
        lv_obj_t* whiteBackground;
        lv_obj_t* bottomPinkBackground;
        lv_obj_t* bottomBlueBackground;
        lv_obj_t* bluetoothStatus;
        lv_obj_t* label_time;
        lv_obj_t* label_date;
        lv_obj_t* label_day;
        lv_obj_t* batteryValue;
        lv_obj_t* stepValue;
        lv_obj_t* notificationIcon;

        Controllers::DateTime& dateTimeController;
        const Controllers::Battery& batteryController;
        const Controllers::Ble bleController;
        Controllers::NotificationManager& notificationManager;
        Controllers::Settings& settingsController;
        Controllers::MotionController& motionController;

        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::Trans> {
      static constexpr WatchFace watchFace = WatchFace::Trans;
      static constexpr const char* name = "Trans";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFaceTrans(controllers.dateTimeController,
                                           controllers.batteryController,
                                           controllers.bleController,
                                           controllers.notificationManager,
                                           controllers.settingsController,
                                           controllers.motionController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      }
    };
  }
}
