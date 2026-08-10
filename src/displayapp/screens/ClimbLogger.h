#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "displayapp/screens/Symbols.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      // Step 1 scaffold: placeholder screen only, proves the app registers
      // and launches. Selection UI (area/colour/grade, result, attempts)
      // is built in Step 2.
      class ClimbLogger : public Screen {
      public:
        ClimbLogger();
        ~ClimbLogger() override;

      private:
        lv_obj_t* label = nullptr;
      };
    }

    template <>
    struct AppTraits<Apps::ClimbLogger> {
      static constexpr Apps app = Apps::ClimbLogger;
      static constexpr const char* icon = Screens::Symbols::shoe;

      static Screens::Screen* Create(AppControllers& /*controllers*/) {
        // Step 3+ will pass filesystem/motor controllers through here once
        // the catalog/log files and haptic confirmation are wired up.
        return new Screens::ClimbLogger();
      }

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      }
    };
  }
}
