#pragma once

#include "displayapp/screens/Screen.h"
#include "components/settings/Settings.h"
#include <components/motion/MotionController.h>
#include <lvgl/lvgl.h>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Dice : public Screen {
      public:
        Dice(DisplayApp* app, Pinetime::Controllers::Settings& settingsController, Controllers::MotionController& motionController);
        ~Dice() override;
        void OnButtonEvent(lv_obj_t* obj, lv_event_t event);
        bool OnButtonPushed() override;
        void Refresh() override;

      private:
        Controllers::Settings& settingsController;
        Controllers::MotionController& motionController;
        uint8_t count, grade, substractor;
        lv_obj_t *txtCount, *txtGrade, *txtGradeSeparator, *txtSubstractor, *txtSubstractorSeparator, *lblampm, *btnCountUp, *btnCountDown,
          *btnGradeUp, *txtGradeUp, *btnGradeDown, *txtGradeDown, *txtCountUp, *txtCountDown, *btnSubstractorUp, *txtSubstractorUp,
          *btnSubstractorDown, *txtSubstractorDown, *btnRoll, *txtRoll;
        lv_obj_t* txtMessage = nullptr;
        lv_obj_t* btnMessage = nullptr;
        lv_task_t* refreshTask;
        void ShowRoll();
        void HideRoll();
        void UpdateRollConfig();
        uint8_t Roll();
      };
    }
  }
}