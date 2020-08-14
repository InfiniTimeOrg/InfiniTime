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
      class ApplicationList : public Screen {
        public:
          explicit ApplicationList(DisplayApp* app);
          ~ApplicationList() override;
          bool Refresh() override;
          bool OnButtonPushed() override;
          bool OnTouchEvent(TouchEvents event) override;
        private:
          bool running = true;

          ScreenList<1> screens;
          std::unique_ptr<Screen> CreateScreen1();
          std::unique_ptr<Screen> CreateScreen2();
          std::unique_ptr<Screen> CreateScreen3();
      };
    }
  }
}