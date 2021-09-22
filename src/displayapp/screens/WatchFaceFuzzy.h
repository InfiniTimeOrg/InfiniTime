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
                Controllers::DateTime& dateTimeController;
                lv_task_t* taskRefresh;
                lv_obj_t* label_time;
                const char* timeAccent = "ffffff";
                static const char* timeSectors[12];
                static const char* hourNames[12];
            };
        }
    }
}

