#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "displayapp/screens/Symbols.h"
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      // A minimal user app: one button, one counter label.
      // An instance is created when the app is opened from the launcher,
      // and destroyed when you leave the app (state is NOT preserved).
      class TapCounter : public Screen {
      public:
        TapCounter();
        ~TapCounter() override;

        // Called from the LVGL event callback when the button is clicked
        void OnButtonClicked();

      private:
        int count = 0;

        lv_obj_t* counterLabel = nullptr;
        lv_obj_t* button = nullptr;
        lv_obj_t* buttonLabel = nullptr;
      };
    }

    // AppTraits is how InfiniTime's build-time app registry learns about
    // this app: its enum id, its launcher icon, and how to construct it.
    // CreateAppDescriptions() in UserApps.h reads this at compile time for
    // every app listed in ENABLE_USERAPPS.
    template <>
    struct AppTraits<Apps::TapCounter> {
      static constexpr Apps app = Apps::TapCounter;
      // Any constant from displayapp/screens/Symbols.h works here
      static constexpr const char* icon = Screens::Symbols::check;

      static Screens::Screen* Create(AppControllers& /*controllers*/) {
        // If your app later needs the date, heart rate, settings, etc.,
        // pass references from `controllers` into your constructor here.
        return new Screens::TapCounter();
      }

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      }
    };
  }
}
