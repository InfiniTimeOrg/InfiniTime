#pragma once

#include <chrono>
#include <cstdint>
#include <memory>
#include "displayapp/Controllers.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Apps.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Controllers {
    class Settings;
    class Battery;
    class Ble;
    class NotificationManager;
    class MotionController;
    class DateTime;
    class HeartRateController;
    class WeatherService;
    class FS;
  }

  namespace Applications {
    namespace Screens {
      class Clock : public Screen {
      public:
        Clock(Controllers::DateTime& dateTimeController,
              const Controllers::Battery& batteryController,
              const Controllers::Ble& bleController,
              Controllers::NotificationManager& notificationManager,
              Controllers::Settings& settingsController,
              Controllers::HeartRateController& heartRateController,
              Controllers::MotionController& motionController,
              Controllers::WeatherService& weatherService,
              Controllers::FS& filesystem);
        ~Clock() override;

        bool OnTouchEvent(TouchEvents event) override;
        bool OnButtonPushed() override;

      private:
        Controllers::DateTime& dateTimeController;
        const Controllers::Battery& batteryController;
        const Controllers::Ble& bleController;
        Controllers::NotificationManager& notificationManager;
        Controllers::Settings& settingsController;
        Controllers::HeartRateController& heartRateController;
        Controllers::MotionController& motionController;
        Controllers::WeatherService& weatherService;
        Controllers::FS& filesystem;

        std::unique_ptr<Screen> screen;
        std::unique_ptr<Screen> WatchFaceDigitalScreen();
        std::unique_ptr<Screen> WatchFaceAnalogScreen();
        std::unique_ptr<Screen> WatchFacePineTimeStyleScreen();
        std::unique_ptr<Screen> WatchFaceTerminalScreen();
        std::unique_ptr<Screen> WatchFaceInfineatScreen();
        std::unique_ptr<Screen> WatchFaceCasioStyleG7710();
      };
    }
  }
}
