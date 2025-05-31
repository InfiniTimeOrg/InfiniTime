#pragma once

#include <displayapp/screens/BatteryIcon.h>
#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "components/ble/SimpleWeatherService.h"
#include "components/ble/BleController.h"
#include "displayapp/widgets/StatusIcons.h"
#include "displayapp/Colors.h"
#include "utility/DirtyValue.h"
#include "displayapp/apps/Apps.h"
#include <displayapp/Controllers.h>
#include "displayapp/screens/BatteryIcon.h"
#include "components/ble/SimpleWeatherService.h"

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
      class WatchFacePhotoStyle : public Screen {
      public:
        WatchFacePhotoStyle(Controllers::DateTime& dateTimeController,
                         const Controllers::Battery& batteryController,
                         const Controllers::Ble& bleController,
                         Controllers::NotificationManager& notificationManager,
                         Controllers::Settings& settingsController,
                         Controllers::HeartRateController& heartRateController,
                         Controllers::MotionController& motionController,
                         Controllers::SimpleWeatherService& weather,
                         Controllers::FS& fs);
        ~WatchFacePhotoStyle() override;

        bool OnTouchEvent(TouchEvents event) override;
        bool OnButtonPushed() override;

        void UpdateSelected(lv_obj_t* object, lv_event_t event);

        void Refresh() override;

      private:
        uint8_t displayedHour = -1;
        uint8_t displayedMinute = -1;

        uint32_t savedTick = 0;

        bool isAvailable = false;

        Utility::DirtyValue<uint8_t> batteryPercentRemaining {};
        Utility::DirtyValue<bool> powerPresent {};
        Utility::DirtyValue<bool> bleState {};
        Utility::DirtyValue<bool> bleRadioEnabled {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>> currentDateTime {};
        Utility::DirtyValue<uint32_t> stepCount {};
        Utility::DirtyValue<uint8_t> heartbeat {};
        Utility::DirtyValue<bool> heartbeatRunning {};
        Utility::DirtyValue<bool> notificationState {};
        Utility::DirtyValue<std::optional<Pinetime::Controllers::SimpleWeatherService::CurrentWeather>> currentWeather {};

        static Pinetime::Controllers::Settings::ContentStyle GetNextContent(Controllers::Settings::ContentStyle content);
        static Pinetime::Controllers::Settings::Colors GetNext(Controllers::Settings::Colors color);
        static Pinetime::Controllers::Settings::Colors GetPrevious(Controllers::Settings::Colors color);

        using days = std::chrono::duration<int32_t, std::ratio<86400>>; // TODO: days is standard in c++20
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, days>> currentDate;

        lv_obj_t* label_time;

        lv_obj_t* btnTopContent;
        lv_obj_t* btnBottomContent;
        lv_obj_t* btnNextBG;
        lv_obj_t* btnPrevBG;
        lv_obj_t* btnNextTime;
        lv_obj_t* btnPrevTime;
        lv_obj_t* btnReset;
        lv_obj_t* btnPhotoToggle;
        lv_obj_t* btnClose;
        lv_obj_t* btnNoPhoto;
        lv_obj_t* backgroundColor;
        lv_obj_t* watchPhoto;
        lv_obj_t* labelDateTop;
        lv_obj_t* labelDateBottom;
        lv_obj_t* heartbeatValueTop;
        lv_obj_t* heartbeatIconBottom;
        lv_obj_t* heartbeatValueBottom;
        lv_obj_t* batteryValueTop;
        lv_obj_t* batteryValueBottom;
        lv_obj_t* batteryIconBottom;
        lv_obj_t* stepIconBottom;
        lv_obj_t* stepValueBottom;
        lv_obj_t* stepValueTop;
        lv_obj_t* statusIcons;
        lv_obj_t* btnSetColor;
        lv_obj_t* btnSetOpts;
        lv_obj_t* temperatureTop;
        lv_obj_t* weatherIconBottom;
        lv_obj_t* temperatureBottom;
        lv_obj_t* lblToggle;

        Controllers::DateTime& dateTimeController;
        Controllers::NotificationManager& notificationManager;
        const Controllers::Ble& bleController;
        const Controllers::Battery& batteryController;
        Controllers::Settings& settingsController;
        Controllers::HeartRateController& heartRateController;
        Controllers::MotionController& motionController;
        Controllers::SimpleWeatherService& weatherService;

        void CloseMenu();
        void RefreshContent();

        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::PhotoStyle> {
      static constexpr WatchFace watchFace = WatchFace::PhotoStyle;
      static constexpr const char* name = "Photo face";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFacePhotoStyle(controllers.dateTimeController,
                                             controllers.batteryController,
                                             controllers.bleController,
                                             controllers.notificationManager,
                                             controllers.settingsController,
                                             controllers.heartRateController,
                                             controllers.motionController,
                                             *controllers.weatherController,
                                             controllers.filesystem);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      }
    };
  }
}