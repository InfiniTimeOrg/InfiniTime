#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include "displayapp/Controllers.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/BatteryIcon.h"
#include "components/datetime/DateTimeController.h"
#include "components/ble/SimpleWeatherService.h"
#include "components/ble/BleController.h"
#include "utility/DirtyValue.h"

constexpr uint8_t PART_COUNT_LIST_ITEM = 3;
constexpr uint8_t PART_COUNT_UPPER_SHAPE = 6;
constexpr uint8_t PART_COUNT_LOWER_SHAPE = 7;

namespace Pinetime {
  namespace Controllers {
    class Settings;
    class Battery;
    class Ble;
    class NotificationManager;
    class HeartRateController;
    class MotionController;
    class WeatherService;
  }

  namespace Applications {
    namespace Screens {

      class WatchFaceStarTrek : public Screen {
      public:
        WatchFaceStarTrek(Controllers::DateTime& dateTimeController,
                          const Controllers::Battery& batteryController,
                          const Controllers::Ble& bleController,
                          Controllers::NotificationManager& notificatioManager,
                          Controllers::Settings& settingsController,
                          Controllers::HeartRateController& heartRateController,
                          Controllers::MotionController& motionController,
                          Controllers::FS& filesystem,
                          Controllers::SimpleWeatherService& weatherService);
        ~WatchFaceStarTrek() override;

        bool OnTouchEvent(TouchEvents event) override;
        bool OnButtonPushed() override;
        void UpdateSelected(lv_obj_t* object, lv_event_t event);

        void Refresh() override;
        void OnLCDWakeup() override;
        void OnLCDSleep() override;

      private:
        Controllers::DateTime& dateTimeController;
        const Controllers::Battery& batteryController;
        const Controllers::Ble& bleController;
        Controllers::NotificationManager& notificationManager;
        Controllers::Settings& settingsController;
        Controllers::HeartRateController& heartRateController;
        Controllers::MotionController& motionController;
        Controllers::FS& filesystem;
        Controllers::SimpleWeatherService& weatherService;

        lv_task_t* taskRefresh;

        uint8_t displayedHour = -1;
        uint8_t displayedMinute = -1;
        uint8_t displayedSecond = -1;
        uint16_t currentYear = 1970;
        Controllers::DateTime::Months currentMonth = Pinetime::Controllers::DateTime::Months::Unknown;
        Controllers::DateTime::Days currentDayOfWeek = Pinetime::Controllers::DateTime::Days::Unknown;
        uint8_t currentDay = 0;

        Utility::DirtyValue<uint8_t> batteryPercentRemaining {};
        Utility::DirtyValue<bool> powerPresent {};
        Utility::DirtyValue<bool> bleState {};
        Utility::DirtyValue<bool> bleRadioEnabled {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> currentDateTime {};
        Utility::DirtyValue<uint32_t> stepCount {};
        Utility::DirtyValue<uint8_t> heartbeat {};
        Utility::DirtyValue<bool> heartbeatRunning {};
        Utility::DirtyValue<bool> notificationState {};
        Utility::DirtyValue<std::optional<Pinetime::Controllers::SimpleWeatherService::CurrentWeather>> currentWeather {};

        lv_obj_t* label_time_hour_1;
        lv_obj_t* label_time_hour_10;
        lv_obj_t* label_time_min_1;
        lv_obj_t* label_time_min_10;
        lv_obj_t* label_time_seconds;
        lv_obj_t* label_time_ampm;
        lv_obj_t* label_dayname;
        lv_obj_t* label_day;
        lv_obj_t* label_month;
        lv_obj_t* label_year;
        lv_obj_t* bleIcon;
        lv_obj_t* batteryPlug;
        lv_obj_t* heartbeatIcon;
        lv_obj_t* heartbeatValue;
        lv_obj_t* stepIcon;
        lv_obj_t* stepValue;
        lv_obj_t* stepBar;
        lv_obj_t* notificationIcon;
        BatteryIcon batteryIcon;
        lv_obj_t* weatherIcon;
        lv_obj_t* temperature;

        // background
        lv_obj_t *topRightRect, *bottomRightRect;
        lv_obj_t *bar1, *bar2;
        lv_obj_t* listItem1[PART_COUNT_LIST_ITEM];
        lv_obj_t* listItem2[PART_COUNT_LIST_ITEM];
        lv_obj_t* listItem3[PART_COUNT_LIST_ITEM];
        lv_obj_t* listItem4[PART_COUNT_LIST_ITEM];
        lv_obj_t* upperShape[PART_COUNT_UPPER_SHAPE];
        lv_obj_t* lowerShape[PART_COUNT_LOWER_SHAPE];
        lv_obj_t* imgBracketLeft;
        lv_obj_t* rectBracketLeft;
        lv_obj_t* imgBracketRight;
        lv_obj_t* rectBracketRight;
        lv_obj_t *minuteAnchor, *hourAnchor;

        // config menu
        lv_obj_t* btnSetUseSystemFont;
        lv_obj_t* lblSetUseSystemFont;
        lv_obj_t* btnSetAnimate;
        lv_obj_t* lblSetAnimate;
        lv_obj_t* btnSetWeather;
        lv_obj_t* lblSetWeather;
        lv_obj_t* btnSetDisplaySeconds;
        lv_obj_t* lblSetDisplaySeconds;
        lv_obj_t* btnClose;
        lv_obj_t* lblClose;
        uint32_t settingsAutoCloseTick = 0;
        bool showingMenu = false;
        void createMenu();
        void destroyMenu();

        // ### visibility functions affect everything BUT the time digits
        // ### a watch should always tell the time ;)

        void drawWatchFace(bool visible = true);
        void setTimeAnchorForDisplaySeconds(bool displaySeconds);
        void realignTime();
        lv_obj_t* rect(bool visible, uint8_t w, uint8_t h, uint8_t x, uint8_t y, lv_color_t color);
        lv_obj_t* circ(bool visible, uint8_t d, uint8_t x, uint8_t y, lv_color_t color);
        lv_obj_t* _base(bool visible, uint8_t w, uint8_t h, uint8_t x, uint8_t y, lv_color_t color);
        lv_obj_t* label(bool visible,
                        lv_color_t color,
                        lv_obj_t* alignto = lv_scr_act(),
                        lv_align_t alignmode = LV_ALIGN_CENTER,
                        int16_t gapx = 0,
                        int16_t gapy = 0,
                        const char* text = "",
                        lv_obj_t* base = lv_scr_act());
        lv_obj_t* button(bool visible,
                         uint16_t sizex,
                         uint16_t sizey,
                         lv_obj_t* alignto = lv_scr_act(),
                         lv_align_t alignmode = LV_ALIGN_CENTER,
                         int16_t gapx = 0,
                         int16_t gapy = 0);

        lv_font_t* font_time = nullptr;
        lv_font_t* font_StarTrek = nullptr;
        bool starTrekFontAvailable = false;
        void updateFontTime();

        bool weatherNeedsRefresh = false;

        uint32_t animatorStartTick = 0;
        uint32_t animatorContinuousTick = 0;
        uint8_t animatorStartStage = 0;
        uint8_t animatorContinuousStage = 0;
        bool startAnimationFinished = false;
        const char* animateMenuButtonText();
        void setVisible(bool visible = true);
        void setShapeVisible(lv_obj_t** shape, uint8_t partcount, bool visible);
        void animateStartStep();
        void startStartAnimation();
        void animateContinuousStep();
        void startContinuousAnimation();
        void resetColors();
        Controllers::Settings::StarTrekAnimateType animateStateCycler(Controllers::Settings::StarTrekAnimateType previous);
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::StarTrek> {
      static constexpr WatchFace watchFace = WatchFace::StarTrek;
      static constexpr const char* name = "Star Trek";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFaceStarTrek(controllers.dateTimeController,
                                              controllers.batteryController,
                                              controllers.bleController,
                                              controllers.notificationManager,
                                              controllers.settingsController,
                                              controllers.heartRateController,
                                              controllers.motionController,
                                              controllers.filesystem,
                                              *controllers.weatherController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS&) {
        return true;
      }
    };
  }
}