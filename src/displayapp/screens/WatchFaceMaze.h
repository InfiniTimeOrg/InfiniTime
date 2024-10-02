#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "components/datetime/DateTimeController.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class WatchFaceMaze : public Screen {
      public:
        WatchFaceMaze(Controllers::DateTime&);
        ~WatchFaceMaze() override;
      };
    }

    template <>
    struct WatchFaceTraits<WatchFace::Maze> {
      static constexpr WatchFace watchFace = WatchFace::Maze;
      static constexpr const char* name = "Maze";

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::WatchFaceMaze(controllers.dateTimeController/*,
                                              controllers.batteryController,
                                              controllers.bleController,
                                              controllers.notificationManager,
                                              controllers.settingsController,
                                              controllers.heartRateController,
                                              controllers.motionController*/);
      };

      static bool IsAvailable(Pinetime::Controllers::FS& /*filesystem*/) {
        return true;
      }
    };
  }
}