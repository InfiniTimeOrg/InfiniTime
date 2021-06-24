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

        bool Refresh() override;

        bool OnButtonPushed() override;

        bool OnTouchEvent(TouchEvents event) override;

      private:
        bool running = true;

        Pinetime::Controllers::DateTime& dateTimeController;
        Controllers::WeatherService& weatherService;

        ScreenList<5> screens;

        std::unique_ptr<Screen> CreateScreenTemperature();

        std::unique_ptr<Screen> CreateScreen2();

        std::unique_ptr<Screen> CreateScreen3();

        std::unique_ptr<Screen> CreateScreen4();

        std::unique_ptr<Screen> CreateScreen5();
      };
    }
  }
}