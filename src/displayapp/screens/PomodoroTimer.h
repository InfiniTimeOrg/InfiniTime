#pragma once

#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Symbols.h"
#include "utility/DirtyValue.h"
#include <lvgl/lvgl.h>
#include <chrono>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class PomodoroTimer : public Screen {
      public:
        PomodoroTimer(AppControllers& controllers);
        ~PomodoroTimer() override;
        void Refresh() override;

      private:
        AppControllers& controllers;
        
        lv_obj_t* lblTime;
        lv_obj_t* lblSessionType;
        lv_obj_t* btnStartPause;
        lv_obj_t* lblStartPause;
        
        // Placeholder for future implementation
        Utility::DirtyValue<std::chrono::seconds> displaySeconds;
      };
    }

    template <>
    struct AppTraits<Apps::PomodoroTimer> {
      static constexpr Apps app = Apps::PomodoroTimer;
      static constexpr const char* icon = Screens::Symbols::clock;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::PomodoroTimer(controllers);
      }

      static bool IsAvailable(Controllers::FS& /*filesystem*/) {
        return true;
      }
    };
  }
}