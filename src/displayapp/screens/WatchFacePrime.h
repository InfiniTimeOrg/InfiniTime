#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "components/ble/SimpleWeatherService.h"
#include "components/ble/BleController.h"
#include "displayapp/widgets/StatusIcons.h"
#include "utility/DirtyValue.h"
#include "displayapp/apps/Apps.h"

namespace Pinetime {
  namespace Controllers {
    class Settings;
    class Battery;
    class Ble;
    class AlarmController;
    class NotificationManager;
    class HeartRateController;
    class MotionController;
    class MusicService;
  }

  namespace Applications {
    namespace Screens {

      class WatchFacePrime : public Screen {
      public:
        WatchFacePrime(Controllers::DateTime& dateTimeController,
                       const Controllers::Battery& batteryController,
                       const Controllers::Ble& bleController,
                       const Controllers::AlarmController& alarmController,
                       Controllers::NotificationManager& notificationManager,
                       Controllers::Settings& settingsController,
                       Controllers::HeartRateController& heartRateController,
                       Controllers::MotionController& motionController,
                       Controllers::SimpleWeatherService& weather,
                       Controllers::MusicService& music);
        ~WatchFacePrime() override;

        void Refresh() override;

      private:
        uint8_t displayedHour = -1;
        uint8_t displayedMinute = -1;

        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>> currentDateTime {};
        Utility::DirtyValue<uint32_t> stepCount {};
        Utility::DirtyValue<uint8_t> heartbeat {};
        Utility::DirtyValue<bool> heartbeatRunning {};
        Utility::DirtyValue<bool> notificationState {};
        Utility::DirtyValue<std::optional<Pinetime::Controllers::SimpleWeatherService::CurrentWeather>> currentWeather {};

        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::days>> currentDate;

        lv_obj_t* label_time;
        lv_obj_t* label_time_ampm;
        lv_obj_t* label_date;
        lv_obj_t* heartbeatIcon;
        lv_obj_t* heartbeatValue;
        lv_obj_t* stepIcon;
        lv_obj_t* stepValue;
        lv_obj_t* notificationIcon;
        lv_obj_t* weatherIcon;
        lv_obj_t* temperature;
        lv_obj_t* label_music;
        lv_obj_t* icon_music;
        lv_obj_t* weatherLabel;
        lv_obj_t* label_battery_value;
        std::string track;

        Controllers::DateTime& dateTimeController;
        Controllers::NotificationManager& notificationManager;
        Controllers::Settings& settingsController;
        Controllers::HeartRateController& heartRateController;
        Controllers::MotionController& motionController;
        Controllers::SimpleWeatherService& weatherService;
        Controllers::MusicService& musicService;

        lv_task_t* taskRefresh;
        Widgets::StatusIcons statusIcons;
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::Prime> {
      static constexpr WatchFace watchFace = WatchFace::Prime;
      static constexpr const char* name = "Prime";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFacePrime(controllers.dateTimeController,
                                           controllers.batteryController,
                                           controllers.bleController,
                                           controllers.alarmController,
                                           controllers.notificationManager,
                                           controllers.settingsController,
                                           controllers.heartRateController,
                                           controllers.motionController,
                                           *controllers.weatherController,
                                           *controllers.musicService);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      }
    };
  }
}
