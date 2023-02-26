#pragma once

#include <memory>
#include <components/ble/weather/WeatherService.h>
#include "Screen.h"
#include "ScreenList.h"

namespace Pinetime {
  namespace Applications {
    class DisplayApp;

    namespace Screens {
      class Weather : public Screen {
      public:
        explicit Weather(DisplayApp* app, Pinetime::Controllers::WeatherService& weather);

        ~Weather() override;

        void Refresh() override;

        bool OnButtonPushed() override;

        bool OnTouchEvent(TouchEvents event) override;

      private:
        DisplayApp* app;
        bool running = true;

        Controllers::WeatherService& weatherService;

        ScreenList<5> screens;

        std::unique_ptr<Screen> CreateScreenTemperature();

        std::unique_ptr<Screen> CreateScreenAir();

        std::unique_ptr<Screen> CreateScreenClouds();

        std::unique_ptr<Screen> CreateScreenPrecipitation();

        std::unique_ptr<Screen> CreateScreenHumidity();
      };
    }
  }
}
