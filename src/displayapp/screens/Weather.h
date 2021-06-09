#pragma once

#include <memory>
#include "Screen.h"
#include "ScreenList.h"

namespace Pinetime {
  namespace Controllers {
    class DateTime;
    class Battery;
    class BrightnessController;
    class Ble;
  }

  namespace Drivers {
    class WatchdogView;
  }

  namespace Applications {
    class DisplayApp;

    namespace Screens {
      class Weather : public Screen {
      public:
        explicit Weather(DisplayApp* app,
                         Pinetime::Controllers::DateTime& dateTimeController,
                         Pinetime::Controllers::Battery& batteryController,
                         Pinetime::Controllers::BrightnessController& brightnessController,
                         Pinetime::Controllers::Ble& bleController,
                         Pinetime::Drivers::WatchdogView& watchdog);
        ~Weather() override;
        bool Refresh() override;
        bool OnButtonPushed() override;
        bool OnTouchEvent(TouchEvents event) override;

      private:
        bool running = true;

        Pinetime::Controllers::DateTime& dateTimeController;
        Pinetime::Controllers::Battery& batteryController;
        Pinetime::Controllers::BrightnessController& brightnessController;
        Pinetime::Controllers::Ble& bleController;
        Pinetime::Drivers::WatchdogView& watchdog;

        ScreenList<5> screens;
        std::unique_ptr<Screen> CreateScreen1();
        std::unique_ptr<Screen> CreateScreen2();
        std::unique_ptr<Screen> CreateScreen3();
        std::unique_ptr<Screen> CreateScreen4();
        std::unique_ptr<Screen> CreateScreen5();
      };
    }
  }
}