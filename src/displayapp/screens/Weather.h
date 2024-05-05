#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "displayapp/screens/Screen.h"
#include "components/ble/SimpleWeatherService.h"
#include "displayapp/apps/Apps.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"
#include "utility/DirtyValue.h"

namespace Pinetime {

  namespace Controllers {
    class Settings;
  }

  namespace Applications {
    namespace Screens {

      class Weather : public Screen {
      public:
        Weather(Controllers::Settings& settingsController, Controllers::SimpleWeatherService& weatherService);
        ~Weather() override;

        void Refresh() override;

      private:
        Controllers::Settings& settingsController;
        Controllers::SimpleWeatherService& weatherService;

        Utility::DirtyValue<std::optional<Controllers::SimpleWeatherService::CurrentWeather>> currentWeather {};
        Utility::DirtyValue<std::optional<Controllers::SimpleWeatherService::Forecast>> currentForecast {};

        lv_obj_t* icon;
        lv_obj_t* condition;
        lv_obj_t* temperature;
        lv_obj_t* minTemperature;
        lv_obj_t* maxTemperature;
        lv_obj_t* forecast;

        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct AppTraits<Apps::Weather> {
      static constexpr Apps app = Apps::Weather;
      static constexpr const char* icon = Screens::Symbols::cloudSunRain;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Weather(controllers.settingsController, *controllers.weatherController);
      };
    };
  }
}
