#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include "utility/DirtyValue.h"
#include "displayapp/apps/Apps.h"
#include "displayapp/Controllers.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class WatchFaceDigital;

      class WatchFaceRailway : public Screen {
      public:
        WatchFaceRailway(AppControllers& controllers);

        ~WatchFaceRailway() override;

        void Refresh() override;
        bool OnTouchEvent(TouchEvents event) override;

      private:
        uint8_t sHour, sMinute;

        Utility::DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds>> currentDateTime;

        // 12 hour notch marks (linemeter)
        lv_obj_t* hourNotchMeter;

        // Hands (linemeter, rotated via angle_offset)
        lv_obj_t* hourHandMeter;
        lv_obj_t* minuteHandMeter;

        // Center dot
        lv_obj_t* centerDot;

        // Digital overlay
        WatchFaceDigital* digitalOverlay;
        lv_task_t* overlayDismissTask;

        AppControllers& controllers;

        void CreateAnalogFace();
        void UpdateClock();
        void ShowOverlay();
        void HideOverlay();
        static void DismissOverlayCallback(lv_task_t* task);

        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::Railway> {
      static constexpr WatchFace watchFace = WatchFace::Railway;
      static constexpr const char* name = "Railway";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFaceRailway(controllers);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      }
    };
  }
}
