#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/ScreenList.h"
#include "components/ble/SimpleWeatherService.h"
#include "displayapp/apps/Apps.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"

namespace Pinetime {

  namespace Controllers {
    class Settings;
  }

  namespace Applications {
    class DisplayApp;

    namespace Screens {

      class Weather : public Screen {
      public:
        Weather(DisplayApp* app,
                Controllers::Settings& settingsController,
                Controllers::SimpleWeatherService& weatherService,
                Controllers::DateTime& dateTimeController);
        ~Weather() override;

        bool OnTouchEvent(TouchEvents event) override;

      private:
        DisplayApp* app;
        Controllers::Settings& settingsController;
        Controllers::SimpleWeatherService& weatherService;
        Controllers::DateTime& dateTimeController;

        ScreenList<3> screens;

        std::unique_ptr<Screen> CreateScreen1();
        std::unique_ptr<Screen> CreateScreen2();
        std::unique_ptr<Screen> CreateScreen3();
      };
    }

    template <>
    struct AppTraits<Apps::Weather> {
      static constexpr Apps app = Apps::Weather;
      static constexpr const char* icon = Screens::Symbols::cloudSunRain;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Weather(controllers.displayApp,
                                    controllers.settingsController,
                                    *controllers.weatherController,
                                    controllers.dateTimeController);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      };
    };
  }
}
