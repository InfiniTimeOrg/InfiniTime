#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include <displayapp/Controllers.h>
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "components/ble/SimpleWeatherService.h"
#include "utility/DirtyValue.h"

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

      class WatchFaceJson : public Screen {
      public:
        WatchFaceJson(Controllers::DateTime& dateTimeController,
                          const Controllers::Battery& batteryController,
                          const Controllers::Ble& bleController,
                          Controllers::NotificationManager& notificationManager,
                          Controllers::Settings& settingsController,
                          Controllers::HeartRateController& heartRateController,
                          Controllers::MotionController& motionController,
                          Controllers::SimpleWeatherService& weather);
        ~WatchFaceJson() override;

        void Refresh() override;

      private:
        Utility::DirtyValue<float> batteryPercentRemaining {};
        Utility::DirtyValue<bool> powerPresent {};
        Utility::DirtyValue<bool> bleState {};
        Utility::DirtyValue<bool> bleRadioEnabled {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>> currentDateTime {};
        Utility::DirtyValue<uint32_t> stepCount {};
        Utility::DirtyValue<uint8_t> heartbeat {};
        Utility::DirtyValue<bool> heartbeatRunning {};
        Utility::DirtyValue<bool> notificationState {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::days>> currentDate;
        Utility::DirtyValue<std::optional<Pinetime::Controllers::SimpleWeatherService::CurrentWeather>> currentWeather {};

        lv_obj_t* notification_icon;

        lv_obj_t* label_open;
        lv_obj_t* label_close;

        lv_obj_t* label_time;
        lv_obj_t* label_date;
        lv_obj_t* label_battery_open;
        lv_obj_t* label_battery_close;
        lv_obj_t* label_battery_percent;
        lv_obj_t* label_battery_status;
        lv_obj_t* label_steps;
        lv_obj_t* label_heartrate;
        lv_obj_t* label_status;
        lv_obj_t* label_weather;
        lv_obj_t* weather_icon;
        lv_obj_t* label_temperature;

        Controllers::DateTime& dateTimeController;
        const Controllers::Battery& batteryController;
        const Controllers::Ble& bleController;
        Controllers::NotificationManager& notificationManager;
        Controllers::Settings& settingsController;
        Controllers::HeartRateController& heartRateController;
        Controllers::MotionController& motionController;
        Controllers::SimpleWeatherService& weatherService;

        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::Json> {
      static constexpr WatchFace watchFace = WatchFace::Json;
      static constexpr const char* name = "Json";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFaceJson(controllers.dateTimeController,
                                              controllers.batteryController,
                                              controllers.bleController,
                                              controllers.notificationManager,
                                              controllers.settingsController,
                                              controllers.heartRateController,
                                              controllers.motionController,
                                              *controllers.weatherController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      }
    };
  }
}
