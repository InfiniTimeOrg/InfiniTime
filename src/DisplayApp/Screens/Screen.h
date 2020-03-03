#pragma once

namespace Pinetime {
  namespace Applications {
    class DisplayApp;
    namespace Screens {
      class Screen {
        public:
          Screen(DisplayApp* app) : app{app} {}
          virtual ~Screen() = default;

          // Return false if the app can be closed, true if it must continue to run
          virtual bool Refresh() = 0;

          // Return false if the button hasn't been handled by the app, true if it has been handled
          virtual bool OnButtonPushed() { return false; }

        protected:
          DisplayApp* app;
      };
    }
  }
}
