#pragma once

#include <vector>
#include "Screen.h"
#include "Label.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class ScreenList : public Screen {
        public:
          explicit ScreenList(DisplayApp* app,
                              Pinetime::Controllers::DateTime& dateTimeController,
                              Pinetime::Controllers::Battery& batteryController,
                              Pinetime::Controllers::BrightnessController& brightnessController,
                              Pinetime::Drivers::WatchdogView& watchdog);
          ~ScreenList() override;
          bool Refresh() override;
          bool OnButtonPushed() override;
          bool OnTouchEvent(TouchEvents event) override;
        private:
          bool running = true;
          uint8_t screenIndex = 0;

          // TODO choose another container without dynamic alloc
          std::vector<Screens::Label> screens;
          Pinetime::Controllers::DateTime& dateTimeController;
          Pinetime::Controllers::Battery& batteryController;
          Pinetime::Controllers::BrightnessController& brightnessController;
          Pinetime::Drivers::WatchdogView& watchdog;

          char t1[200];
          char t2[30];
          char t3[42];
      };
    }
  }
}