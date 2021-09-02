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
      class SystemInfo : public Screen {
      public:
        explicit SystemInfo(DisplayApp* app,
                            Pinetime::Controllers::DateTime& dateTimeController,
                            Pinetime::Controllers::Battery& batteryController,
                            Pinetime::Controllers::BrightnessController& brightnessController,
                            Pinetime::Controllers::Ble& bleController,
                            Pinetime::Drivers::WatchdogView& watchdog,
                            Pinetime::Controllers::MotionController& motionController);
        ~SystemInfo() override;
        bool OnTouchEvent(TouchEvents event) override;

      private:
        Pinetime::Controllers::DateTime& dateTimeController;
        Pinetime::Controllers::Battery& batteryController;
        Pinetime::Controllers::BrightnessController& brightnessController;
        Pinetime::Controllers::Ble& bleController;
        Pinetime::Drivers::WatchdogView& watchdog;
        Pinetime::Controllers::MotionController& motionController;

        ScreenList<5> screens;

        static bool sortById(const TaskStatus_t& lhs, const TaskStatus_t& rhs);

        std::unique_ptr<Screen> CreateScreen1();
        std::unique_ptr<Screen> CreateScreen2();
        std::unique_ptr<Screen> CreateScreen3();
        std::unique_ptr<Screen> CreateScreen4();
        std::unique_ptr<Screen> CreateScreen5();
      };
    }
  }
}
