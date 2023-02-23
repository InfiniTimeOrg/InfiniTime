#pragma once

#include <memory>
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/ScreenList.h"

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
                            const Pinetime::Controllers::Battery& batteryController,
                            Pinetime::Controllers::BrightnessController& brightnessController,
                            Pinetime::Controllers::Ble& bleController,
                            Pinetime::Drivers::WatchdogView& watchdog,
                            Pinetime::Controllers::MotionController& motionController,
                            Pinetime::Drivers::Cst816S& touchPanel);
        ~SystemInfo() override;
        bool OnTouchEvent(TouchEvents event) override;

      private:
        Pinetime::Controllers::DateTime& dateTimeController;
        const Pinetime::Controllers::Battery& batteryController;
        Pinetime::Controllers::BrightnessController& brightnessController;
        Pinetime::Controllers::Ble& bleController;
        Pinetime::Drivers::WatchdogView& watchdog;
        Pinetime::Controllers::MotionController& motionController;
        Pinetime::Drivers::Cst816S& touchPanel;

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
