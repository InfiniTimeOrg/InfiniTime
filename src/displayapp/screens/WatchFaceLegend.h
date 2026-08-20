#pragma once

#include <lvgl/lvgl.h>
#include <chrono>
#include <cstdint>
#include <optional>
#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
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

      class WatchFaceLegend : public Screen {
      public:
        WatchFaceLegend(Controllers::DateTime& dateTimeController,
                        const Controllers::Battery& batteryController,
                        const Controllers::Ble& bleController,
                        const Controllers::AlarmController& alarmController,
                        Controllers::Timer& timerController,
                        Controllers::StopWatchController& stopWatchController,
                        Controllers::NotificationManager& notificationManager,
                        Controllers::Settings& settingsController,
                        Controllers::HeartRateController& heartRateController,
                        Controllers::MotionController& motionController,
                        Controllers::SimpleWeatherService& weatherService);
        ~WatchFaceLegend() override;

        void Refresh() override;

      private:
        Utility::DirtyValue<uint8_t> batteryPercentRemaining {};
        Utility::DirtyValue<bool> powerPresent {};
        Utility::DirtyValue<bool> bleState {};
        Utility::DirtyValue<bool> bleRadioEnabled {};
        Utility::DirtyValue<bool> alarmEnabled {};
        Utility::DirtyValue<bool> timerRunning {};
        Utility::DirtyValue<bool> stopWatchRunning {};
        Utility::DirtyValue<Controllers::Settings::Notification> notificationConfig {};
        Utility::DirtyValue<bool> notificationState {};
        Utility::DirtyValue<bool> notificationEmpty {};
        Utility::DirtyValue<uint32_t> stepCount {};
        Utility::DirtyValue<uint8_t> heartbeat {};
        Utility::DirtyValue<bool> heartbeatRunning {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>> currentDateTime {};
        Utility::DirtyValue<std::optional<Controllers::SimpleWeatherService::CurrentWeather>> currentWeather {};

        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::days>> currentDate;

        // Tracks whether weather is displayed to adjust the layout only when necessary
        bool weatherDisplayed = false;

        // Top status strip
        lv_obj_t* notificationAnyDot;
        lv_obj_t* notificationNewIcon;
        lv_obj_t* servicesIcons;
        lv_obj_t* bleIcon;
        lv_obj_t* batteryValue;
        lv_obj_t* upperDivider;

        // Center block
        lv_obj_t* labelDate;
        lv_obj_t* labelTime;
        lv_obj_t* labelAmPm;
        lv_obj_t* divider;

        // Bottom row
        lv_obj_t* stepValue;
        lv_obj_t* stepCaption;
        lv_obj_t* heartbeatValue;
        lv_obj_t* heartbeatCaption;
        lv_obj_t* weatherValue;
        lv_obj_t* weatherCaption;
        lv_obj_t* weatherIcon;

        static constexpr lv_color_t colorForeground = LV_COLOR_MAKE(0xF5, 0xF4, 0xEF);
        static constexpr lv_color_t colorSecondary = LV_COLOR_MAKE(0x8A, 0x8A, 0x85);
        static constexpr lv_color_t colorCaption = LV_COLOR_MAKE(0x4A, 0x4A, 0x46);
        static constexpr lv_color_t colorDivider = LV_COLOR_MAKE(0x2A, 0x2A, 0x2A);
        static constexpr lv_color_t colorAnyNotification = LV_COLOR_MAKE(0xEF, 0x9D, 0x3A);
        static constexpr lv_color_t colorRed = LV_COLOR_MAKE(0xE8, 0x43, 0x3C);
        static constexpr lv_color_t colorBlue = LV_COLOR_MAKE(0x37, 0x8A, 0xDD);
        static constexpr lv_color_t colorCharging = LV_COLOR_MAKE(0x0A, 0xAD, 0x50);
        static constexpr lv_color_t colorWeatherDefault = LV_COLOR_MAKE(0xA6, 0xAC, 0xB0);
        static constexpr lv_color_t colorWeatherYellow = LV_COLOR_MAKE(0xF4, 0xC5, 0x42);
        static constexpr lv_color_t colorWeatherLightBlue = LV_COLOR_MAKE(0x82, 0xD3, 0xE8);

        lv_point_t dividerPoints[2] {{0, 0}, {232, 0}};

        Controllers::DateTime& dateTimeController;
        const Controllers::Battery& batteryController;
        const Controllers::Ble& bleController;
        const Controllers::AlarmController& alarmController;
        Controllers::Timer& timerController;
        Controllers::StopWatchController& stopWatchController;
        Controllers::NotificationManager& notificationManager;
        Controllers::Settings& settingsController;
        Controllers::HeartRateController& heartRateController;
        Controllers::MotionController& motionController;
        Controllers::SimpleWeatherService& weatherService;

        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::Legend> {
      static constexpr WatchFace watchFace = WatchFace::Legend;
      static constexpr const char* name = "Legend";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFaceLegend(controllers.dateTimeController,
                                            controllers.batteryController,
                                            controllers.bleController,
                                            controllers.alarmController,
                                            controllers.timer,
                                            controllers.stopWatchController,
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
