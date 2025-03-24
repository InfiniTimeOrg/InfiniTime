#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "displayapp/screens/Screen.h"
#include <components/motion/MotionController.h>
#include "displayapp/apps/Apps.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"

namespace Pinetime {

  namespace Controllers {
    class Settings;
  }

  namespace Applications {
    namespace Screens {

      class Steps : public Screen {
      public:
        Steps(Controllers::MotionController& motionController, Controllers::Settings& settingsController);
        ~Steps() override;

        void Refresh() override;
        void lapBtnEventHandler(lv_event_t event);

      private:
        Controllers::MotionController& motionController;
        Controllers::Settings& settingsController;

        uint32_t currentTripSteps = 0;

        lv_obj_t* lSteps;
        lv_obj_t* stepsArc;
        lv_obj_t* resetBtn;
        lv_obj_t* resetButtonLabel;
        lv_obj_t* tripLabel;

        uint32_t stepsCount;

        lv_task_t* taskRefresh;
      };
    }

    template <>
    struct AppTraits<Apps::Steps> {
      static constexpr Apps app = Apps::Steps;
      static constexpr const char* icon = Screens::Symbols::shoe;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Steps(controllers.motionController, controllers.settingsController);
      };
    };
  }
}
