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
        Utility::DirtyValue<uint8_t> batteryPercentRemaining {};
        Utility::DirtyValue<bool> powerPresent {};
        Utility::DirtyValue<bool> bleState {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>> currentDateTime {};
        Utility::DirtyValue<uint32_t> stepCount {};
        Utility::DirtyValue<bool> notificationState {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::days>> currentDate;
        // Must be wrapped in a dirty value, since it is displayed in the day but is updated twice a day
        Utility::DirtyValue<const char*> ampmChar {"AM"};

        lv_obj_t* topBlueBackground;
        lv_obj_t* topPinkBackground;
        lv_obj_t* whiteBackground;
        lv_obj_t* bottomPinkBackground;
        lv_obj_t* bottomBlueBackground;
        lv_obj_t* bluetoothStatus;
        lv_obj_t* labelTime;
        lv_obj_t* labelDate;
        lv_obj_t* labelDay;
        lv_obj_t* batteryValue;
        lv_obj_t* stepValue;
        lv_obj_t* notificationIcon;

        static constexpr lv_color_t lightBlue = LV_COLOR_MAKE(0x00, 0xbf, 0xf3);
        static constexpr lv_color_t lightPink = LV_COLOR_MAKE(0xf4, 0x9a, 0xc1);

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
      static constexpr const char* name = "Trans Flag";

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
