#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "components/settings/Settings.h"
#include "displayapp/screens/Screen.h"
#include <components/motion/MotionController.h>
#include "systemtask/SystemTask.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class SettingShakeThreshold : public Screen {
      public:
        SettingShakeThreshold(Pinetime::Controllers::Settings& settingsController,
                              Controllers::MotionController& motionController,
                              System::SystemTask& systemTask);

        ~SettingShakeThreshold() override;
        void Refresh() override;
        void UpdateSelected(lv_obj_t* object, lv_event_t event);

      private:
        Controllers::Settings& settingsController;
        Controllers::MotionController& motionController;
        System::SystemTask& systemTask;
        uint8_t calibrating;
        bool EnableForCal;
        uint32_t vDecay, vCalTime;
        lv_obj_t *positionArc, *animArc, *calButton, *calLabel;
        lv_task_t* refreshTask;
      };
    }
  }
}
