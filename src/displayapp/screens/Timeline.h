#pragma once

#include <lvgl/lvgl.h>
#include "components/calendar/CalendarManager.h"

#include "Screen.h"
#include "Clock.h"

#define CANVAS_WIDTH  192
#define CANVAS_Y_POSITION 60
#define CANVAS_HEIGHT 180

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Timeline : public Screen {
      public:
        explicit Timeline(DisplayApp* app, Controllers::DateTime& dateTimeController, Controllers::CalendarManager& calendarManager);
        ~Timeline() override;
        bool OnButtonPushed() override;
        bool OnTouchEvent(TouchEvents event) override;
        bool OnTouchEvent(uint16_t x, uint16_t y) override;

      private:
        void displayCurrentEvent();
        void displayWeek();
  
        Controllers::DateTime& dateTimeController;
        Controllers::CalendarManager& calendarManager;
        Controllers::CalendarManager::CalendarEventIterator  currentEvent;
        
        bool running = true;
        uint8_t  today_index = 0;
        
        lv_obj_t * title_label;
        lv_obj_t * time_label;
        lv_obj_t * days_label;
        lv_obj_t * hours_label;
        lv_obj_t * week_canvas;
        uint8_t cbuf[LV_IMG_BUF_SIZE_INDEXED_1BIT(CANVAS_WIDTH, CANVAS_HEIGHT)]{};
      };
    }
  }
}
