#pragma once

#include <Components/Gfx/Gfx.h>

namespace Pinetime {
  namespace Applications {
    class DisplayApp;
    namespace Screens {
      class Screen {
        public:
          enum class NextScreen {None, Clock, Menu, App};
          Screen(DisplayApp* app, Components::Gfx& gfx) : app{app}, gfx{gfx} {}
          virtual ~Screen() = default;
          virtual void Refresh(bool fullRefresh) = 0;
          NextScreen GetNextScreen() {return nextScreen;}
          virtual void OnButtonPushed() {};

        protected:
          DisplayApp* app;
          Components::Gfx& gfx;
          NextScreen nextScreen = NextScreen::None;
      };
    }
  }
}
