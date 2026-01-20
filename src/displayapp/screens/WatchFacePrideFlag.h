#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <vector>
#include <cstdint>
#include <array>
#include <FreeRTOS.h>
#include "displayapp/screens/Screen.h"
#include "utility/DirtyValue.h"
#include "components/settings/Settings.h"
#include "components/battery/BatteryController.h"

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

      class WatchFacePrideFlag : public Screen {
      public:
        WatchFacePrideFlag(Controllers::DateTime& dateTimeController,
                           const Controllers::Battery& batteryController,
                           const Controllers::Ble& bleController,
                           Controllers::NotificationManager& notificationManager,
                           Controllers::Settings& settingsController,
                           Controllers::MotionController& motionController);
        ~WatchFacePrideFlag() override;

        bool OnTouchEvent(TouchEvents event) override;
        bool OnButtonPushed() override;

        void Refresh() override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        void UpdateScreen(Pinetime::Controllers::Settings::PrideFlag);

        Utility::DirtyValue<uint8_t> batteryPercentRemaining;
        Utility::DirtyValue<bool> powerPresent;
        Utility::DirtyValue<bool> bleState;
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>> currentDateTime;
        Utility::DirtyValue<uint32_t> stepCount;
        Utility::DirtyValue<bool> notificationState;
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::days>> currentDate;
        // Must be wrapped in a dirty value, since it is displayed in the day but is updated twice a day
        Utility::DirtyValue<const char*> ampmChar {"AM"};

        TickType_t savedTick = 0;

        std::vector<lv_obj_t*> backgroundSections;
        bool themeChanged = false;
        lv_obj_t* bluetoothStatus;
        lv_obj_t* labelTime;
        lv_obj_t* labelDate;
        lv_obj_t* labelDay;
        lv_obj_t* batteryValue;
        lv_obj_t* stepValue;
        lv_obj_t* notificationText;
        lv_obj_t* btnClose;
        lv_obj_t* btnNextFlag;
        lv_obj_t* btnPrevFlag;

        Controllers::DateTime& dateTimeController;
        const Controllers::Battery& batteryController;
        const Controllers::Ble& bleController;
        Controllers::NotificationManager& notificationManager;
        Controllers::Settings& settingsController;
        Controllers::MotionController& motionController;

        lv_task_t* taskRefresh;
        void CloseMenu();
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::PrideFlag> {
      static constexpr WatchFace watchFace = WatchFace::PrideFlag;
      static constexpr const char* name = "Pride Flag";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFacePrideFlag(controllers.dateTimeController,
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
