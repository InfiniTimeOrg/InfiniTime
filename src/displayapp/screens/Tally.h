#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "Symbols.h"

#include <array>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Tally : public Screen {
      public:
        Tally(Controllers::MotionController& motionController,
              Controllers::MotorController& motorController,
              Controllers::Settings& settingsController);
        ~Tally() override;
        void Refresh() override;
        void Increment();
        void Reset();
        void ToggleShakeToCount();

      private:
        static const int SHAKE_DELAY_TIME = 300; // milliseconds
        lv_obj_t* countButton;
        lv_obj_t* countLabel;
        lv_obj_t* resetButton;
        lv_obj_t* resetLabel;
        lv_obj_t* shakeButton;
        lv_obj_t* shakeLabel;
        int count = 0;
        bool shakeToCountEnabled = false;
        bool shakeToWakeTempEnable = false;
        int shakeToCountDelay = 0;
        void UpdateCount();
        void ShakeToWakeEnable();
        void ShakeToWakeDisable();

        Controllers::MotionController& motionController;
        Controllers::MotorController& motorController;
        Controllers::Settings& settingsController;
        lv_task_t* refreshTask;
      };
    }

    template <>
    struct AppTraits<Apps::Tally> {
      static constexpr Apps app = Apps::Tally;
      static constexpr const char* icon = Screens::Symbols::tally;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Tally(controllers.motionController, controllers.motorController, controllers.settingsController);
      };
    };
  }
}
