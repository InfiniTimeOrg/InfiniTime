#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
/* #include <chrono> */
/* #include <cstdint> */
/* #include <memory> */
#include "Screen.h"
//#include "ScreenList.h"
#include "components/datetime/DateTimeController.h"

namespace Pinetime {
    namespace Applications {
        namespace Screens {
            class WatchFaceFuzzy : public Screen {
                public:
                WatchFaceFuzzy(DisplayApp* app, Controllers::DateTime& dateTimeController);
                ~WatchFaceFuzzy() override;
                void Refresh() override;

                private:
                lv_obj_t* label_time;
                Controllers::DateTime& dateTimeController;
                lv_task_t* taskRefresh;
            };
        }
    }
}

