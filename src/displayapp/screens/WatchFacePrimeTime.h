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

      class WatchFacePrimeTime : public Screen {
      public:
        WatchFacePrimeTime(Controllers::DateTime& dateTimeController,
                           const Controllers::Battery& batteryController,
                           const Controllers::Ble& bleController,
                           const Controllers::AlarmController& alarmController,
                           Controllers::NotificationManager& notificationManager,
                           Controllers::Settings& settingsController,
                           Controllers::HeartRateController& heartRateController,
                           Controllers::MotionController& motionController,
                           Controllers::SimpleWeatherService& weather,
                           Controllers::MusicService& music,
                           Controllers::FS& filesystem);
        ~WatchFacePrimeTime() override;

        void Refresh() override;

        static bool IsAvailable(Pinetime::Controllers::FS& filesystem);

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

        lv_font_t* font_primetime = nullptr;

        lv_task_t* taskRefresh;
        Widgets::StatusIcons statusIcons;
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::PrimeTime> {
      static constexpr WatchFace watchFace = WatchFace::PrimeTime;
      static constexpr const char* name = "PrimeTime";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFacePrimeTime(controllers.dateTimeController,
                                               controllers.batteryController,
                                               controllers.bleController,
                                               controllers.alarmController,
                                               controllers.notificationManager,
                                               controllers.settingsController,
                                               controllers.heartRateController,
                                               controllers.motionController,
                                               *controllers.weatherController,
                                               *controllers.musicService,
                                               controllers.filesystem);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& filesystem) {
        return Screens::WatchFacePrimeTime::IsAvailable(filesystem);
      }
    };
  }
}
