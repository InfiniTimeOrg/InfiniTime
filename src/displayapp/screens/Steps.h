#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "Screen.h"
#include <components/motion/MotionController.h>

namespace Pinetime {

  namespace Controllers {
    class Settings;
  }

  namespace Applications {
    namespace Screens {

      class Steps : public Screen {
      public:
        Steps(DisplayApp* app, Controllers::MotionController& motionController, Controllers::Settings& settingsController);
        ~Steps() override;

        void Refresh() override;
        void lapBtnEventHandler(lv_event_t event);

      private:
        Controllers::MotionController& motionController;
        Controllers::Settings& settingsController;

        uint32_t currentLapSteps = 0;

        lv_obj_t* lSteps;
        lv_obj_t* lStepsIcon;
        lv_obj_t* stepsArc;
        lv_obj_t* btnLap;
        lv_obj_t* txtLap;
        lv_obj_t* totalStepsText;

        uint32_t stepsCount;

        lv_task_t* taskRefresh;
      };
    }
  }
}
