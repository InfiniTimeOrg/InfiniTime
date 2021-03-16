#include "Timeline.h"

#include <ctime>
#include <date/date.h>

using namespace Pinetime::Applications::Screens;

static const char days[] = "M\nT\nW\nT\nF\nS\nS\nM\nT\nW\nT\nF\nS";

void formatDateTime(char* buffer, time_t timestamp) {
  auto *time = localtime(&timestamp);
  auto year = 1900 + time->tm_year;
  auto month = 1 + time->tm_mon;
  sprintf(buffer, "%d-%d-%d %d:%d", year, month, time->tm_mday, time->tm_hour, time->tm_min);
}

Timeline::Timeline(DisplayApp* app, Controllers::DateTime& dateTimeController, Controllers::CalendarManager& calendarManager)
  : Screen(app), dateTimeController(dateTimeController), calendarManager(calendarManager) {
  week_canvas = lv_canvas_create(lv_scr_act(), nullptr);
  lv_canvas_set_buffer(week_canvas, &cbuf[0], CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_INDEXED_1BIT);
  lv_obj_set_pos(week_canvas, 20, CANVAS_Y_POSITION);
  lv_canvas_set_palette(week_canvas, 0, LV_COLOR_BLACK);
  lv_canvas_set_palette(week_canvas, 1, LV_COLOR_GRAY);
  lv_color_t c0;
  c0.full = 0;
  lv_canvas_fill_bg(week_canvas, c0, LV_OPA_COVER);
  
  title_label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(title_label, "TIMELINE");
  lv_obj_set_auto_realign(title_label, true);
  lv_obj_align(title_label, nullptr, LV_ALIGN_IN_TOP_MID, 0, 0);

  time_label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(time_label, "Swipe to use!\n\nUp/Down: prev/next\nLeft: timetable\nRight: timeline");
  lv_obj_set_auto_realign(time_label, true);
  lv_obj_align(time_label, nullptr, LV_ALIGN_CENTER, 0, 0);

  days_label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(days_label, "");
  lv_obj_set_auto_realign(days_label, true);
  lv_obj_align(days_label, week_canvas, LV_ALIGN_OUT_LEFT_TOP, -5, 0);

  hours_label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(hours_label, "");
  lv_obj_set_auto_realign(hours_label, true);
  lv_obj_align(hours_label, week_canvas, LV_ALIGN_OUT_TOP_LEFT, 0, 0);
  
  currentEvent = calendarManager.begin();
}

Timeline::~Timeline() {
  lv_obj_clean(lv_scr_act());
}

bool Timeline::OnButtonPushed() {
  running = false;
  return true;
}

bool Timeline::OnTouchEvent(TouchEvents event) {
  switch (event) {
    case TouchEvents::SwipeUp:
      currentEvent++;
      if (currentEvent == calendarManager.end()) {
        currentEvent--;
      }

      displayCurrentEvent();
    break;

    case TouchEvents::SwipeDown:
      if (currentEvent != calendarManager.begin()) {
        currentEvent--;
      }

      displayCurrentEvent();
    break;

    case TouchEvents::SwipeLeft:
      displayWeek();
    break;

    case TouchEvents::SwipeRight:
      displayCurrentEvent();
      break;

    default:
      return false;
  }
  return true;
}

bool Timeline::OnTouchEvent(uint16_t x, uint16_t y) {
  return Screen::OnTouchEvent(x, y);
}

void Timeline::displayWeek() {
  lv_label_set_text_static(title_label, "Next 7 days");
  lv_label_set_text_static(time_label, "");
  
  auto currentDateTime = dateTimeController.CurrentDateTime();
  auto dp = date::floor<date::days>(currentDateTime);
  auto time = date::make_time(currentDateTime-dp);
  auto yearMonthDay = date::year_month_day(dp);
  today_index = date::weekday(yearMonthDay).iso_encoding() - 1;
  
  char days_label_content[14];
  strncpy(&days_label_content[0], &days[today_index*2], 13);
  days_label_content[13] = '\0';
  lv_label_set_text(days_label, &days_label_content[0]);
  lv_label_set_text_static(hours_label, "0 4 8 12 16 20 24");
  
  lv_color_t event_color;
  event_color.full = 1;
  
  for(auto & it : calendarManager) {
    time_t timestamp = it.timestamp;
    auto *it_time = localtime(&timestamp);
    auto day_index = it_time->tm_wday-1;
    auto line_index = day_index - today_index;
    if (line_index == 0 && timestamp > currentDateTime.time_since_epoch().count()) {
      continue; // Do not show events of the same day as today but next week
    }
    
    if (line_index < 0) {
      line_index += 7;
    }
    
    auto y_event = 24*line_index;
    auto begin = it_time->tm_hour*60 + it_time->tm_min;
    auto end = (begin*60 + it.duration) / 60;
    for (uint32_t x = begin/8; x < end/8; ++x) {
      for (auto y_offset = 0; y_offset < 10; ++y_offset) {
        lv_canvas_set_px(week_canvas, x, 5+y_event+y_offset, event_color);
      }
    }
  }
}

void Timeline::displayCurrentEvent() {
  lv_color_t c0;
  c0.full = 0;
  lv_canvas_fill_bg(week_canvas, c0, LV_OPA_COVER);
  lv_label_set_text_static(hours_label, "");
  lv_label_set_text_static(days_label, "");
  
  if (currentEvent == calendarManager.end()) {
    lv_label_set_text(title_label, "");
    lv_label_set_text(time_label, "No event");
  } else {
    auto event = *currentEvent;
    lv_label_set_text_fmt(title_label, "%s", event.title.c_str());

    char begin[16];
    formatDateTime(&begin[0], event.timestamp);
    char end[16];
    formatDateTime(&end[0], event.timestamp + event.duration);
    lv_label_set_text_fmt(time_label, "%s\n->\n%s", &begin[0], &end[0]);
  }
}
