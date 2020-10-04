#pragma once

#include <cstdint>
#include "../TouchEvents.h"

namespace Pinetime {
  namespace Applications {
    class DisplayApp;
    namespace Screens {
      class Screen {
        public:
          explicit Screen(DisplayApp* app) : app{app} {}
          virtual ~Screen() = default;

          // Return false if the app can be closed, true if it must continue to run
          virtual bool Refresh() = 0;

          // Return false if the button hasn't been handled by the app, true if it has been handled
          virtual bool OnButtonPushed() { return false; }

          // Return false if the event hasn't been handled by the app, true if it has been handled
          virtual bool OnTouchEvent(TouchEvents event) { return false; }
          virtual bool OnTouchEvent(uint16_t x, uint16_t y) { return false; }

        protected:
          DisplayApp* app;
      };
    }
  }
}
