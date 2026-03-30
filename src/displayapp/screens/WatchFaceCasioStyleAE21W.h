#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include <displayapp/Controllers.h>
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/BatteryIcon.h"
#include "components/datetime/DateTimeController.h"
#include "components/ble/SimpleWeatherService.h"
#include "components/ble/BleController.h"
#include "displayapp/widgets/StatusIcons.h"
#include "utility/DirtyValue.h"
#include "components/battery/BatteryController.h"
#include "components/ble/NotificationManager.h"

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

            class WatchFaceCasioStyleAE21W : public Screen {
            public:
                WatchFaceCasioStyleAE21W(Controllers::DateTime& dateTimeController,
                                         const Controllers::Battery& batteryController,
                                         const Controllers::Ble& bleController,
                                         Controllers::NotificationManager& notificationManager,
                                         Controllers::Settings& settingsController,
                                         Controllers::HeartRateController& heartRateController,
                                         Controllers::MotionController& motionController,
                                         Controllers::FS& filesystem,
                                         Controllers::SimpleWeatherService& weather);
                ~WatchFaceCasioStyleAE21W() override;

                bool OnTouchEvent(TouchEvents event) override;
                bool OnButtonPushed() override;
                void UpdateSelected(lv_obj_t* object, lv_event_t event);
                void CloseMenu();
                void Refresh() override;

                static bool IsAvailable(Pinetime::Controllers::FS& filesystem);

            private:
                TickType_t savedTick = 0;

                Utility::DirtyValue<uint8_t> batteryPercentRemaining {};
                Utility::DirtyValue<bool> isCharging {};
                Utility::DirtyValue<bool> bleState {};
                Utility::DirtyValue<bool> bleRadioEnabled {};
                Utility::DirtyValue<uint32_t> stepCount {};
                Utility::DirtyValue<uint8_t> heartbeat {};
                Utility::DirtyValue<bool> heartbeatRunning {};
                Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>> currentDateTime {};
                Utility::DirtyValue<bool> notificationState {};
                Utility::DirtyValue<std::optional<Pinetime::Controllers::SimpleWeatherService::CurrentWeather>> currentWeather {};
                Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::days>> currentDate;

                // styles
                lv_style_t style_bg;
                lv_style_t style_lcd_bg;
                lv_style_t style_lcd;

                // G1 Clock
                lv_point_t hour_point[2];
                lv_point_t minute_point[2];
                lv_point_t minute_point_trace[2];
                lv_obj_t* hour_body;
                lv_obj_t* minute_body;
                lv_obj_t* minute_body_trace;

                // G2 seconds
                lv_obj_t* G2SecondMeter = nullptr;

                // Main Drawing
                lv_obj_t* someLvObj;
                lv_obj_t* bg;
                lv_obj_t* graph2MainDisc;
                lv_obj_t* graph2SmallDisc;

                // Graph image
                static lv_img_dsc_t CasioAE21WGraphImage;
                lv_obj_t* AE21WGraph1 = nullptr;
                lv_obj_t* AE21WGraph2 = nullptr;

                // Sec Label Image
                static lv_img_dsc_t SecLabelImage;
                lv_obj_t* secLabel = nullptr;

                // Graph2 top scale
                static lv_img_dsc_t Graph2TopScaleImage;
                lv_obj_t* graph2TopScale = nullptr;

                // Labels and icons
                lv_obj_t* label_function;
                lv_obj_t* label_time;
                lv_obj_t* label_seconds;
                lv_obj_t* label_time_ampm;
                lv_obj_t* label_date;
                lv_obj_t* label_day_of_week;
                lv_obj_t* bleIcon;
                lv_obj_t* plugIcon;
                lv_obj_t* label_battery_value;
                lv_obj_t* heartbeatIcon;
                lv_obj_t* heartbeatValue;
                lv_obj_t* stepIcon;
                lv_obj_t* stepValue;
                lv_obj_t* notificationIcon;
                lv_obj_t* weatherIcon;
                lv_obj_t* temperature;

                BatteryIcon batteryIcon;

                // Settings
                lv_obj_t* labelBtnSettings;
                lv_obj_t* btnClose;
                lv_obj_t* btnNextTheme;
                lv_obj_t* btnPrevTheme;
                lv_obj_t* btnSettings;

                Controllers::DateTime& dateTimeController;
                const Controllers::Battery& batteryController;
                const Controllers::Ble& bleController;
                Controllers::NotificationManager& notificationManager;
                Controllers::Settings& settingsController;
                Controllers::HeartRateController& heartRateController;
                Controllers::MotionController& motionController;
                Controllers::SimpleWeatherService& weatherService;

                void SetBatteryIcon();

                lv_task_t* taskRefresh;
                lv_font_t* font_TechnologyBR_20 = nullptr;
                lv_font_t* font_TechnologyBR_40 = nullptr;
                lv_font_t* font_TechnologyBR_75 = nullptr;
            };
        }

        template <>
        struct WatchFaceTraits<WatchFace::CasioStyleAE21W> {
            static constexpr WatchFace watchFace = WatchFace::CasioStyleAE21W;
            static constexpr const char* name = "Casio AE21W";

            static Screens::Screen* Create(AppControllers& controllers) {
                return new Screens::WatchFaceCasioStyleAE21W(controllers.dateTimeController,
                                                             controllers.batteryController,
                                                             controllers.bleController,
                                                             controllers.notificationManager,
                                                             controllers.settingsController,
                                                             controllers.heartRateController,
                                                             controllers.motionController,
                                                             controllers.filesystem,
                                                             *controllers.weatherController);
            };

            static bool IsAvailable(Pinetime::Controllers::FS& filesystem) {
                return Screens::WatchFaceCasioStyleAE21W::IsAvailable(filesystem);
            }
        };
    }
}
