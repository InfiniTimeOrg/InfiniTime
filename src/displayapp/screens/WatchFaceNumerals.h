#pragma once

#include <lvgl/lvgl.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include <displayapp/Controllers.h>
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "utility/DirtyValue.h"
#include "displayapp/apps/Apps.h"

namespace Pinetime {
  namespace Controllers {
    class Settings;
    class NotificationManager;
  }

  namespace Applications {
    namespace Screens {

      class WatchFaceNumerals : public Screen {
      public:
        WatchFaceNumerals(Controllers::DateTime& dateTimeController,
                          Controllers::NotificationManager& notificationManager,
                          Controllers::Settings& settingsController,
                          Controllers::FS& fs);
        ~WatchFaceNumerals() override;

        void Refresh() override;

        static bool IsAvailable(Pinetime::Controllers::FS& filesystem);

      private:
        uint8_t displayedHour = -1;
        uint8_t displayedMinute = -1;

        uint16_t currentYear = 1970;
        Controllers::DateTime::Months currentMonth = Pinetime::Controllers::DateTime::Months::Unknown;
        Controllers::DateTime::Days currentDayOfWeek = Pinetime::Controllers::DateTime::Days::Unknown;
        uint8_t currentDay = 0;
        Utility::DirtyValue<bool> notificationState {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> currentDateTime {};

        lv_obj_t* label_time_hour;
        lv_obj_t* label_time_minute;
        lv_obj_t* label_time_ampm;
        lv_obj_t* label_date;
        lv_obj_t* notificationIcon;
        lv_obj_t* dateDay;
        lv_obj_t* dateDayOfWeek;

        Controllers::DateTime& dateTimeController;
        Controllers::NotificationManager& notificationManager;
        Controllers::Settings& settingsController;

        lv_font_t* font_large = nullptr;
        lv_font_t* font_small = nullptr;

        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::Numerals> {
      static constexpr WatchFace watchFace = WatchFace::Numerals;
      static constexpr const char* name = "Numeral face";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFaceNumerals(controllers.dateTimeController,
                                              controllers.notificationManager,
                                              controllers.settingsController,
                                              controllers.filesystem);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& filesystem) {
        return Screens::WatchFaceNumerals::IsAvailable(filesystem);
      }
    };
  }
}
