#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <lvgl/src/lv_core/lv_obj.h>
#include <memory>
#include "components/ble/BleController.h"
#include "components/datetime/DateTimeController.h"
#include "displayapp/screens/Screen.h"

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

      class WatchFaceGravel : public Screen {
      public:
        WatchFaceGravel(DisplayApp* app,
                        Controllers::DateTime& dateTimeController,
                        Controllers::Battery& batteryController,
                        Controllers::Settings& settingsController,
                        Controllers::MotionController& motionController,
                        Controllers::FS& filesystem);
        ~WatchFaceGravel() override;

        void Refresh() override;

        static bool IsAvailable(Pinetime::Controllers::FS& filesystem);

      private:
        uint8_t displayedHour = -1;
        uint8_t displayedMinute = -1;
        Controllers::DateTime::Months currentMonth = Pinetime::Controllers::DateTime::Months::Unknown;
        uint8_t currentDay = 0;
        std::array<lv_point_t, 7> stepLinePoints = {0};

        DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> currentDateTime {};
        DirtyValue<bool> motionSensorOk {};
        DirtyValue<uint32_t> stepCount {};
        DirtyValue<uint8_t> batteryPercentRemaining {};
        DirtyValue<bool> isCharging {};

        lv_obj_t* labelTime;
        lv_obj_t* labelTimeAMPM;
        lv_style_t styleStepLine;
        lv_obj_t* stepLine;
        lv_obj_t* stepLineBackground;
        lv_obj_t* labelDate;
        lv_obj_t* batteryBar;
        lv_anim_t animationBatteryCharging;

        lv_font_t* font_SquareRegular72 = nullptr;
        lv_font_t* font_SquareRegular20 = nullptr;

        Controllers::DateTime& dateTimeController;
        Controllers::Settings& settingsController;
        Controllers::MotionController& motionController;
        Controllers::Battery& batteryController;

        lv_task_t* taskRefresh;

        enum class Place { TOP, RIGHT, BOTTOM, LEFT };
        typedef struct {
          Place place;
          float location;
        } DrawStop;

        DrawStop getDrawEnd(float percent, uint16_t width, uint16_t height);
        uint16_t refreshStepLinePoints(uint32_t stepCount);
        void updateBatteryBar(uint8_t percent);
        void toggleBatteryChargingAnimation(bool enabled, uint8_t percent);
      };
    }
  }
}
