#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "displayapp/screens/Screen.h"
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
        void UpdateSelected(lv_obj_t *object, lv_event_t event);


      private:
        Controllers::MotionController& motionController;
        Controllers::Settings& settingsController;

        uint32_t currentTripSteps = 0;

        lv_obj_t* lSteps;
        lv_obj_t* lStepsIcon;
        lv_obj_t* lstepsL;
        lv_obj_t* lstepsGoal;
        lv_obj_t* stepsArc;
        lv_obj_t* btnReset;
        lv_obj_t* btnResetLabel;
        lv_obj_t* btnSet;
        lv_obj_t* btnSetLabel;
        lv_obj_t* btnPlus;
        lv_obj_t* btnMinus;
        lv_obj_t* btnClose;
        lv_obj_t* tripLabel;

        uint32_t stepsCount;

        lv_task_t* taskRefresh;
      };
    }
  }
}
