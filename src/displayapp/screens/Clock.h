#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include <components/heartrate/HeartRateController.h>
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"

namespace Pinetime {
  namespace Controllers {
    class Settings;
    class Battery;
    class Ble;
    class NotificationManager;
    class MotionController;
  }

  namespace Applications {
    namespace Screens {
      class Clock : public Screen {
      public:
        Clock(DisplayApp* app,
              Controllers::DateTime& dateTimeController,
              Controllers::Battery& batteryController,
              Controllers::Ble& bleController,
              Controllers::NotificationManager& notificatioManager,
              Controllers::Settings& settingsController,
              Controllers::HeartRateController& heartRateController,
              Controllers::MotionController& motionController,
              Controllers::FS& fs);
        ~Clock() override;

        bool OnTouchEvent(TouchEvents event) override;
        bool OnButtonPushed() override;

      private:
        Controllers::DateTime& dateTimeController;
        Controllers::Battery& batteryController;
        Controllers::Ble& bleController;
        Controllers::NotificationManager& notificatioManager;
        Controllers::Settings& settingsController;
        Controllers::HeartRateController& heartRateController;
        Controllers::MotionController& motionController;
        Controllers::FS& fs;

        std::unique_ptr<Screen> screen;
        std::unique_ptr<Screen> WatchFaceDigitalScreen();
        std::unique_ptr<Screen> WatchFaceAnalogScreen();
        std::unique_ptr<Screen> WatchFacePineTimeStyleScreen();
        std::unique_ptr<Screen> WatchFaceTerminalScreen();
        std::unique_ptr<Screen> WatchFaceInfineatScreen();
      };
    }
  }
}
