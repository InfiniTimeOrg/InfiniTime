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
  }

  namespace Applications {
    namespace Screens {

      class WatchFaceBinary : public Screen {
      public:
        WatchFaceBinary(Controllers::DateTime& dateTimeController,
                        Controllers::Settings& settingsController,
                        const Controllers::Battery& batteryController,
                        const Controllers::Ble& bleController,
                        const Controllers::AlarmController& alarmController,
                        Controllers::NotificationManager& notificationManager,
                        Controllers::HeartRateController& heartRateController,
                        Controllers::MotionController& motionController,
                        Controllers::SimpleWeatherService& weather);
        ~WatchFaceBinary() override;

        void Refresh() override;

      private:
        lv_obj_t* labelTime;
        lv_obj_t* labelDate;
        lv_obj_t* heartbeatIcon;
        lv_obj_t* heartbeatValue;
        lv_obj_t* stepIcon;
        lv_obj_t* stepValue;
        lv_obj_t* notificationIcon;
        lv_obj_t* weatherIcon;
        lv_obj_t* temperature;

        lv_style_t styleLine;
        const lv_point_t hmLine_points[2] = {{83, 84}, {83, 148}};
        const lv_point_t msLine_points[2] = {{155, 84}, {155, 148}};
        lv_obj_t* hmLine;
        lv_obj_t* msLine;

        lv_obj_t* leds[6][4] = {nullptr};
        lv_obj_t* pmLed = nullptr;

        lv_task_t* taskRefresh = nullptr;

        static const uint8_t COL_START_Y = 160;
        static const uint8_t COL_START_X = 200;
        static const uint8_t COL_SPACING = 36;
        static const uint8_t ROW_SPACING = 36;
        static const uint8_t LED_SIZE = 18;

        const lv_color_t LED_ON_COLOR = LV_COLOR_MAKE(0xFF, 0x00, 0x00);
        const lv_color_t LED_OFF_COLOR = LV_COLOR_MAKE(0x50, 0x00, 0x00);
        const lv_color_t PM_LED_ON_COLOR = LV_COLOR_MAKE(0x00, 0x00, 0xFF);
        const lv_color_t PM_LED_OFF_COLOR = LV_COLOR_MAKE(0x00, 0x00, 0x50);

        // right to left
        const uint8_t MAX_ROWS[6] = {4, 3, 4, 3, 4, 2};

        lv_obj_t* CreateLed(lv_obj_t* parent, lv_coord_t posX, lv_coord_t posY, lv_coord_t size, lv_color_t color);
        void CreateTimeLeds();

        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>> currentDateTime {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::days>> currentDate;
        Utility::DirtyValue<uint32_t> stepCount {};
        Utility::DirtyValue<uint8_t> heartbeat {};
        Utility::DirtyValue<bool> heartbeatRunning {};
        Utility::DirtyValue<bool> notificationState {};
        Utility::DirtyValue<std::optional<Pinetime::Controllers::SimpleWeatherService::CurrentWeather>> currentWeather {};

        Controllers::DateTime& dateTimeController;
        Controllers::Settings& settingsController;
        Controllers::NotificationManager& notificationManager;
        Controllers::HeartRateController& heartRateController;
        Controllers::MotionController& motionController;
        Controllers::SimpleWeatherService& weatherService;

        Widgets::StatusIcons statusIcons;
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::Binary> {
      static constexpr WatchFace watchFace = WatchFace::Binary;
      static constexpr const char* name = "Binary";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFaceBinary(controllers.dateTimeController,
                                            controllers.settingsController,
                                            controllers.batteryController,
                                            controllers.bleController,
                                            controllers.alarmController,
                                            controllers.notificationManager,
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
