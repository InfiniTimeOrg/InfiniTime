#pragma once

#include <Components/Gfx/Gfx.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Screen {
        public:
          Screen(Components::Gfx& gfx) : gfx{gfx} {}
          virtual void Refresh(bool fullRefresh) = 0;

        protected:
          Components::Gfx& gfx;
      };
    }
  }
}
