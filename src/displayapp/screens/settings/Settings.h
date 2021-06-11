#pragma once

#include <cstdint>
#include "displayapp/screens/Screen.h"
#include "displayapp/screens/ScreenList.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class Settings : public Screen {
      public:
        Settings(DisplayApp* app, Pinetime::Controllers::Settings& settingsController);
        ~Settings() override;

        bool Refresh() override;

        bool OnTouchEvent(Pinetime::Applications::TouchEvents event) override;

      private:
        Controllers::Settings& settingsController;

        ScreenList<3> screens;

        std::unique_ptr<Screen> CreateScreen1();
        std::unique_ptr<Screen> CreateScreen2();
        std::unique_ptr<Screen> CreateScreen3();
      };
    }
  }
}
