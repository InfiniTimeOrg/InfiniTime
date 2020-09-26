#pragma once

#include <vector>
#include <Components/Ble/NimbleController.h>
#include "Screen.h"
#include "Label.h"
#include "ScreenList.h"
#include "Gauge.h"
#include "Meter.h"
#include <functional>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class SystemInfo : public Screen {
        public:
          explicit SystemInfo(DisplayApp* app,
                              Pinetime::Controllers::DateTime& dateTimeController,
                              Pinetime::Controllers::Battery& batteryController,
                              Pinetime::Controllers::BrightnessController& brightnessController,
                              Pinetime::Controllers::Ble& bleController,
                              Pinetime::Drivers::WatchdogView& watchdog);
          ~SystemInfo() override;
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

          char t1[200];
          char t2[200];
          char t3[30];

          ScreenList<3> screens;
          std::unique_ptr<Screen> CreateScreen1();
          std::unique_ptr<Screen> CreateScreen2();
          std::unique_ptr<Screen> CreateScreen3();
      };
    }
  }
}