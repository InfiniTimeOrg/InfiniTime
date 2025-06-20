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
#include "displayapp/widgets/Flower.h"
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

      class WatchFaceGarden : public Screen {
      public:
        WatchFaceGarden(Controllers::DateTime& dateTimeController,
                         const Controllers::Battery& batteryController,
                         const Controllers::Ble& bleController,
                         const Controllers::AlarmController& alarmController,
                         Controllers::NotificationManager& notificationManager,
                         Controllers::Settings& settingsController,
                         Controllers::HeartRateController& heartRateController,
                         Controllers::MotionController& motionController,
                         Controllers::SimpleWeatherService& weather);
        ~WatchFaceGarden() override;

        void Refresh() override;

      private:
        static const int NUM_FLOWERS = 5;
        static const int NUM_STAGES = 6;
        static const int GARDEN_WIDTH = 180;
        static constexpr int X_POS_OFFSET = -GARDEN_WIDTH/2;
        static constexpr int X_POS_STEP = GARDEN_WIDTH/(NUM_FLOWERS-1);

        static constexpr uint32_t BG_COLORS[NUM_STAGES*2] = {
          0x000000, 0x16004C,
          0x080065, 0xC6B431,
          0x00539C, 0xABDFFF,
          0x2D7DFF, 0x87BBFF,
          0x2D7DFF, 0x87BBFF,
          0x090463, 0xBB112D
        };

        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>> currentDateTime {};
        Utility::DirtyValue<uint32_t> stepCount {};
        Utility::DirtyValue<uint8_t> heartbeat {};
        Utility::DirtyValue<bool> heartbeatRunning {};
        Utility::DirtyValue<bool> notificationState {};
        Utility::DirtyValue<std::optional<Pinetime::Controllers::SimpleWeatherService::CurrentWeather>> currentWeather {};
        Utility::DirtyValue<int> stage {};
        Utility::DirtyValue<int> seed {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::days>> currentDate;

        lv_obj_t* garden;
        lv_obj_t* label_time_hour;
        lv_obj_t* label_time_min;
        lv_obj_t* label_time_colon;
        lv_obj_t* label_time_ampm;
        lv_obj_t* label_date;
        lv_obj_t* heartbeatIcon;
        lv_obj_t* heartbeatValue;
        lv_obj_t* stepIcon;
        lv_obj_t* stepValue;
        lv_obj_t* notificationIcon;
        lv_obj_t* weatherIcon;
        lv_obj_t* temperature;

        Controllers::DateTime& dateTimeController;
        Controllers::NotificationManager& notificationManager;
        Controllers::Settings& settingsController;
        Controllers::HeartRateController& heartRateController;
        Controllers::MotionController& motionController;
        Controllers::SimpleWeatherService& weatherService;

        lv_task_t* taskRefresh;
        Widgets::StatusIcons statusIcons;
        Widgets::Flower flowers[NUM_FLOWERS];
        uint16_t seeds[NUM_FLOWERS];
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::Garden> {
      static constexpr WatchFace watchFace = WatchFace::Garden;
      static constexpr const char* name = "Garden";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFaceGarden(controllers.dateTimeController,
                                            controllers.batteryController,
                                            controllers.bleController,
                                            controllers.alarmController,
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
