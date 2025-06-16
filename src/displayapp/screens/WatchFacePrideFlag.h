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

        void UpdateScreen(Pinetime::Controllers::Settings::PrideFlag);

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

        static Pinetime::Controllers::Settings::PrideFlag GetNext(Controllers::Settings::PrideFlag prideFlag);
        static Pinetime::Controllers::Settings::PrideFlag GetPrevious(Controllers::Settings::PrideFlag prideFlag);

        uint32_t savedTick = 0;
        bool initialized = false;

        lv_obj_t** backgroundSections;
        char labelTimeColour[8] = "#000000";
        char defaultTopLabelColour[8] = "#ffffff";
        char defaultBottomLabelColour[8] = "#ffffff";
        bool themeChanged = false;
        uint8_t numBackgrounds;
        lv_obj_t* bluetoothStatus;
        lv_obj_t* labelTime;
        lv_obj_t* labelDate;
        lv_obj_t* labelDay;
        lv_obj_t* batteryValue;
        lv_obj_t* stepValue;
        lv_obj_t* notificationIcon;
        lv_obj_t* btnClose;
        lv_obj_t* btnNextFlag;
        lv_obj_t* btnPrevFlag;

        static constexpr lv_color_t lightBlue = LV_COLOR_MAKE(0x00, 0xbf, 0xf3);
        static constexpr lv_color_t lightPink = LV_COLOR_MAKE(0xf4, 0x9a, 0xc1);
        static constexpr lv_color_t hotPink = LV_COLOR_MAKE(0xd6, 0x02, 0x70);
        static constexpr lv_color_t grayPurple = LV_COLOR_MAKE(0x9b, 0x4f, 0x96);
        static constexpr lv_color_t darkBlue = LV_COLOR_MAKE(0x00, 0x38, 0xa8);
        static constexpr lv_color_t orange = LV_COLOR_MAKE(0xef, 0x76, 0x27);
        static constexpr lv_color_t lightOrange = LV_COLOR_MAKE(0xff, 0x9b, 0x55);
        static constexpr lv_color_t lightPurple = LV_COLOR_MAKE(0xd4, 0x61, 0xa6);
        static constexpr lv_color_t darkPurple = LV_COLOR_MAKE(0xb5, 0x56, 0x90);
        static constexpr lv_color_t magenta = LV_COLOR_MAKE(0xa5, 0x00, 0x62);
        static constexpr lv_color_t darkGreen = LV_COLOR_MAKE(0x07, 0x8d, 0x70);
        static constexpr lv_color_t cyan = LV_COLOR_MAKE(0x26, 0xce, 0xaa);
        static constexpr lv_color_t lightGreen = LV_COLOR_MAKE(0x98, 0xe8, 0xc1);
        static constexpr lv_color_t indigo = LV_COLOR_MAKE(0x50, 0x49, 0xcc);
        static constexpr lv_color_t steelBlue = LV_COLOR_MAKE(0x3d, 0x1a, 0x78);

        Controllers::DateTime& dateTimeController;
        const Controllers::Battery& batteryController;
        const Controllers::Ble bleController;
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
