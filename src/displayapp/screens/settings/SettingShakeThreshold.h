#pragma once

#include <cstdint>
#include <lvgl/lvgl.h>
#include "displayapp/screens/Screen.h"

namespace Pinetime::Controllers {
  class Settings;
  class MotionController;
}
namespace Pinetime::System {
  class SystemTask;
}

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

        enum class CalibrationStep {
          NotCalibrated = 0,
          GettingReady = 1,
          Calibrating = 2,
          // Special case for disabled calibration due to sleep mode
          Disabled = 255,
        };

        CalibrationStep currentCalibrationStep = CalibrationStep::NotCalibrated;
        bool oldWakeupModeShake = false;
        uint32_t vDecay = 0, vCalTime = 0;
        lv_obj_t *positionArc = nullptr, *animArc = nullptr, *calButton = nullptr, *calLabel = nullptr;
        lv_task_t* refreshTask = nullptr;
      };
    }
  }
}
