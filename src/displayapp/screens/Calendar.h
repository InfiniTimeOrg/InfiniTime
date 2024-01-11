#pragma once

#include "displayapp/apps/Apps.h"
#include "displayapp/Controllers.h"
#include "displayapp/screens/Screen.h"
#include "components/datetime/DateTimeController.h"
#include <lvgl/lvgl.h>
                                   
#include "Symbols.h"

namespace Pinetime {
  namespace Controllers {
    class Settings;
  }

  namespace Applications {
    namespace Screens {
      class Calendar : public Screen {
      public:
        Calendar(Controllers::DateTime& dateTimeController);
        ~Calendar() override;
      private:
        bool OnTouchEvent(TouchEvents event);
        Controllers::DateTime& dateTimeController;
        lv_obj_t* label_time;
        lv_obj_t  * calendar;
        lv_calendar_date_t today;
        lv_calendar_date_t current;
      };
    }

    template <>
    struct AppTraits<Apps::Calendar> {
      static constexpr Apps app = Apps::Calendar;
      static constexpr const char* icon = Screens::Symbols::calendar;

      static Screens::Screen* Create(AppControllers& controllers) {
        return new Screens::Calendar(controllers.dateTimeController);
      };
    };
  }
}
