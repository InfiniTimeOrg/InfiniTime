#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include <chrono>
#include <cstdint>
#include <memory>
#include "displayapp/screens/Pawn.h"
#include "utility/DirtyValue.h"
#include "displayapp/apps/Apps.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      class WatchFaceDigital : public Pawn {
      public:
        WatchFaceDigital(AppControllers& controllers);
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::Digital> {
      static constexpr WatchFace watchFace = WatchFace::Digital;
      static constexpr const char* name = "Digital";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFaceDigital(controllers);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      }
    };
  }
}
