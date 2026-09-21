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
        static constexpr lv_color_t ampmColor = LV_COLOR_MAKE(0x99, 0x99, 0x99);

        Utility::DirtyValue<bool> notificationState {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::minutes>> currentDateTime {};
        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::days>> currentDate;

        lv_obj_t* labelTimeHour;
        lv_obj_t* labelTimeMinute;
        lv_obj_t* labelTimeAMPM1;
        lv_obj_t* labelTimeAMPM2;
        lv_obj_t* label_date;
        lv_obj_t* notificationIcon;
        lv_obj_t* dateDay;
        lv_obj_t* dateDayOfWeek;

        Controllers::DateTime& dateTimeController;
        Controllers::NotificationManager& notificationManager;
        Controllers::Settings& settingsController;

        lv_font_t* fontLarge = nullptr;
        lv_font_t* fontSmall = nullptr;

        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::Numerals> {
      static constexpr WatchFace watchFace = WatchFace::Numerals;
      static constexpr const char* name = "Numerals";

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
