#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/screens/Screen.h"
#include "displayapp/Controllers.h"
#include "systemtask/SystemTask.h"
#include "systemtask/WakeLock.h"
#include "Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class SleepTracker : public Screen {
      public:
        SleepTracker(Controllers::HeartRateController& HeartRateController, Controllers::DateTime& DateTimeController, System::SystemTask& systemTask);
        ~SleepTracker() override;

        void Refresh() override;

        void GetBPM();


      private:
        Controllers::HeartRateController& heartRateController;
        Controllers::DateTime& dateTimeController;
        Pinetime::System::WakeLock wakeLock;

        int bpm = 0;
        int prevBpm = 0;
        int rollingBpm = 0;

        lv_obj_t* label_hr;

        lv_task_t* mainRefreshTask;
        lv_task_t* hrRefreshTask;
      };
    }
    
    template <>
    struct AppTraits<Apps::SleepTracker> {
      static constexpr Apps app = Apps::SleepTracker;
      static constexpr const char* icon = Screens::Symbols::bed;
      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::SleepTracker(controllers.heartRateController, controllers.dateTimeController, *controllers.systemTask);
      }
    };
  }
}