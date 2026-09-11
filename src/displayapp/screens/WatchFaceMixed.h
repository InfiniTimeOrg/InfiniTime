#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include "displayapp/screens/Screen.h"
#include "displayapp/widgets/StatusIcons.h"
#include "utility/DirtyValue.h"

namespace Pinetime {
  namespace Controllers {
    class Settings;
    class Battery;
    class Ble;
    class NotificationManager;
    class AlarmController;
    class HeartRateController;
    class MotionController;
    class SimpleWeatherService;
  }

  namespace Applications {
    namespace Screens {

      class WatchFaceMixed : public Screen {
      public:
        WatchFaceMixed(Controllers::DateTime& dateTimeController,
                       const Controllers::Battery& batteryController,
                       const Controllers::Ble& bleController,
                       const Controllers::AlarmController& alarmController,
                       Controllers::NotificationManager& notificationManager,
                       Controllers::Settings& settingsController,
                       Controllers::HeartRateController& heartRateController,
                       Controllers::MotionController& motionController,
                       Controllers::SimpleWeatherService& weather);

        ~WatchFaceMixed() override;

        void Refresh() override;

      private:
        uint8_t sHour, sMinute, sSecond;

        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> currentDateTime;
        Utility::DirtyValue<uint32_t> stepCount {};
        Utility::DirtyValue<uint8_t> heartbeat {};
        Utility::DirtyValue<bool> heartbeatRunning {};
        Utility::DirtyValue<bool> notificationState {false};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::days>> currentDate;
        Utility::DirtyValue<std::optional<Pinetime::Controllers::SimpleWeatherService::CurrentWeather>> currentWeather {};

        lv_obj_t* minor_scales;
        lv_obj_t* major_scales;
        lv_obj_t* large_scales;
        lv_obj_t* twelve;

        lv_obj_t* hour_body;
        lv_obj_t* hour_body_trace;
        lv_obj_t* minute_body;
        lv_obj_t* minute_body_trace;
        lv_obj_t* second_body;

        lv_point_t hour_point[2];
        lv_point_t hour_point_trace[2];
        lv_point_t minute_point[2];
        lv_point_t minute_point_trace[2];
        lv_point_t second_point[2];

        lv_style_t hour_line_style;
        lv_style_t hour_line_style_trace;
        lv_style_t minute_line_style;
        lv_style_t minute_line_style_trace;
        lv_style_t second_line_style;

        lv_obj_t* label_time;
        lv_obj_t* label_time_ampm;
        lv_obj_t* heartbeatIcon;
        lv_obj_t* heartbeatValue;
        lv_obj_t* stepIcon;
        lv_obj_t* stepValue;
        lv_obj_t* label_date;
        lv_obj_t* notificationIcon;
        lv_obj_t* weatherIcon;
        lv_obj_t* temperature;

        Controllers::DateTime& dateTimeController;
        Controllers::NotificationManager& notificationManager;
        Controllers::Settings& settingsController;
        Controllers::HeartRateController& heartRateController;
        Controllers::MotionController& motionController;
        Controllers::SimpleWeatherService& weatherService;
        Widgets::StatusIcons statusIcons;

        void UpdateClock();
        void UpdateDate();
        void UpdateHeartbeat();
        void UpdateSteps();
        void UpdateWeather();

        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::Mixed> {
      static constexpr WatchFace watchFace = WatchFace::Mixed;
      static constexpr const char* name = "Mixed face";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFaceMixed(controllers.dateTimeController,
                                           controllers.batteryController,
                                           controllers.bleController,
                                           controllers.alarmController,
                                           controllers.notificationManager,
                                           controllers.settingsController,
                                           controllers.heartRateController,
                                           controllers.motionController,
                                           *controllers.weatherController);
      }

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      }
    };
  }
}
