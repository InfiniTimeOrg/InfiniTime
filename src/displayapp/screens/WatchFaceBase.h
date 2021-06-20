#pragma once

#include "Screen.h"
#include "../DisplayApp.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class WatchFaceBase : public Screen {
      public:
        explicit WatchFaceBase(DisplayApp* app);

        virtual ~WatchFaceBase() = default;

      private:
      };
    }
  }
}

