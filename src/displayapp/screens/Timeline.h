#pragma once

#include "components/calendar/CalendarManager.h"
#include "displayapp/LittleVgl.h"
#include <lvgl/lvgl.h>

#include "Clock.h"
#include "Screen.h"

// Timetable hours(y-axis) formatting
static constexpr uint16_t MINUTES_PER_DAY      = 24 * 60;
static constexpr uint16_t MINUTES_PER_TIMESLOT = 20;
static constexpr uint16_t TIMESLOT_PX_HEIGHT   = 3;
static constexpr lv_coord_t TIMETABLE_HEIGHT   = MINUTES_PER_DAY / MINUTES_PER_TIMESLOT * TIMESLOT_PX_HEIGHT;

// Timetable days(x-axis) formatting
static constexpr lv_coord_t TIMESLOT_PX_WIDTH = 16;
static constexpr lv_coord_t INTER_DAY_SPACE   = 8;
static constexpr lv_coord_t DAY_PX_OFFSET     = INTER_DAY_SPACE + TIMESLOT_PX_WIDTH;
static constexpr lv_coord_t TIMETABLE_WIDTH   = 7 * DAY_PX_OFFSET + INTER_DAY_SPACE;

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Timeline : public Screen {
      public:
        explicit Timeline(DisplayApp* app, Controllers::DateTime& dateTimeController, Controllers::CalendarManager& calendarManager, Components::LittleVgl& lvgl);
        ~Timeline() override;
        bool OnTouchEvent(TouchEvents event) override;
        bool OnTouchEvent(uint16_t x, uint16_t y) override;

      private:
        void displayCurrentEvent();
        void set_timeslot_area(lv_area_t* timeslot_area, uint8_t day_index, uint16_t timeslot_index);
        void draw_event(uint8_t day_column, uint16_t event_start, uint16_t duration);
        void displayWeek();

        Controllers::DateTime& dateTimeController;
        Controllers::CalendarManager& calendarManager;
        Controllers::CalendarManager::CalendarEventIterator currentEvent;

        uint8_t  today_index = 0;

        lv_obj_t * title_label;
        lv_obj_t * time_label;

        lv_obj_t * days_container;
        lv_obj_t* hours_labels[5]; // 4 8 12 16 20
        lv_obj_t * hours_container;
        lv_obj_t* days_labels[7];

        Pinetime::Components::LittleVgl& lvgl;
        static constexpr uint16_t TIMESLOT_BUFFER_SIZE = TIMESLOT_PX_WIDTH * TIMESLOT_PX_HEIGHT;
        lv_color_t timeslotBuffer[TIMESLOT_BUFFER_SIZE];
        lv_obj_t * timetable_draw_area;
      };
    }
  }
}
