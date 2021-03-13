#pragma once

#include <lvgl/src/lv_core/lv_obj.h>
#include "components/calendar/CalendarManager.h"

#include "Screen.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Timeline : public Screen {
      public:
        explicit Timeline(DisplayApp* app, Controllers::CalendarManager& calendarManager);
        ~Timeline() override;
        bool OnButtonPushed() override;
        bool OnTouchEvent(TouchEvents event) override;
        bool OnTouchEvent(uint16_t x, uint16_t y) override;

      private:
        void displayCurrent();

        Controllers::CalendarManager& calendarManager;
        Controllers::CalendarManager::CalendarEventIterator  currentEvent;
        bool running = true;
        lv_obj_t * title_label;
        lv_obj_t * time_label;
      };
    }
  }
}
