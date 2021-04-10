#pragma once
#include <FreeRTOS.h>
#include <task.h>
#include <cstdint>
#include <lvgl/lvgl.h>
#include <timers.h>
#include "Screen.h"

namespace Pinetime {

  namespace Applications {
    namespace Screens {

      class Tasks : public Screen{
        public:
          Tasks(DisplayApp* app);
          ~Tasks() override;

          bool Refresh() override;
          void UpdateScreen();
         
        private:
          mutable TaskStatus_t tasksStatus[7];

          lv_task_t* taskUpdate;
          lv_obj_t * table;

      };
    }
  }
}
