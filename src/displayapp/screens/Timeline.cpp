#include "Timeline.h"

#include <ctime>
#include <date/date.h>

using namespace Pinetime::Applications::Screens;

static constexpr char days[] = "M\0T\0W\0T\0F\0S\0S\0M\0T\0W\0T\0F\0S";
static constexpr char hours_texts[] = " 4\0" " 8\0" "12\0" "16\0" "20";

void formatDateTime(char* buffer, time_t timestamp) {
  auto *time = localtime(&timestamp);
  auto year = 1900 + time->tm_year;
  auto month = 1 + time->tm_mon;
  sprintf(buffer, "%d-%d-%d %d:%d", year, month, time->tm_mday, time->tm_hour, time->tm_min);
}

Timeline::Timeline(DisplayApp* app,
                   Controllers::DateTime& dateTimeController,
                   Controllers::CalendarManager& calendarManager,
                   Components::LittleVgl& lvgl)
  : Screen(app), dateTimeController(dateTimeController), calendarManager(calendarManager), lvgl{lvgl} {
  title_label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(title_label, "TIMELINE");
  lv_obj_set_auto_realign(title_label, true);
  lv_obj_align(title_label, nullptr, LV_ALIGN_IN_TOP_MID, 0, 0);

  time_label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(time_label, "Swipe to use!\n\nUp/Down: prev/next\nLeft: timetable\nRight: timeline");
  lv_obj_set_auto_realign(time_label, true);
  lv_obj_align(time_label, nullptr, LV_ALIGN_CENTER, 0, 0);

  days_container = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_opa(days_container, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_size(days_container, TIMETABLE_WIDTH, lv_obj_get_height(title_label));
  lv_obj_align(days_container, nullptr, LV_ALIGN_IN_TOP_MID, 0, 0);

  timetable_draw_area = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_size(timetable_draw_area, TIMETABLE_WIDTH + 1, TIMETABLE_HEIGHT + 1);
  lv_obj_set_style_local_radius(timetable_draw_area, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_bg_opa(timetable_draw_area, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_bg_color(timetable_draw_area, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_TRANSP);
  lv_obj_set_style_local_border_color(timetable_draw_area, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_set_style_local_border_width(timetable_draw_area, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 1);
  lv_obj_move_background(timetable_draw_area);
  lv_obj_align(timetable_draw_area, days_container, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

  for (int i=0; i < 7; i++) {
    days_labels[i] = lv_label_create(days_container, nullptr);
    lv_label_set_text_static(days_labels[i], &days[i*2]);
    lv_obj_set_width(days_labels[i], TIMESLOT_PX_WIDTH);
    lv_obj_align_mid_x(days_labels[i], nullptr, LV_ALIGN_IN_BOTTOM_LEFT, INTER_DAY_SPACE + TIMESLOT_PX_WIDTH/2 + DAY_PX_OFFSET*i);
    lv_obj_align_y(days_labels[i], nullptr, LV_ALIGN_IN_BOTTOM_LEFT, 0);
  }

  hours_container = lv_obj_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_bg_opa(hours_container, LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_size(hours_container, 25, TIMETABLE_HEIGHT);
  lv_obj_align(hours_container, timetable_draw_area, LV_ALIGN_OUT_LEFT_MID, 0, 0);

  static constexpr lv_coord_t HOURS_LABELS_SPACING = TIMETABLE_HEIGHT / 6;
  for (int i=0; i < 5; i++) {
    hours_labels[i] = lv_label_create(hours_container, nullptr);
    lv_label_set_text_static(hours_labels[i], &hours_texts[i*3]);
    lv_obj_align_mid_x(hours_labels[i], nullptr, LV_ALIGN_IN_TOP_MID, 0);
    lv_obj_align_mid_y(hours_labels[i], nullptr, LV_ALIGN_IN_TOP_MID, HOURS_LABELS_SPACING*(i+1));
  }

  lv_obj_set_hidden(timetable_draw_area, true);
  lv_obj_set_hidden(days_container, true);
  lv_obj_set_hidden(hours_container, true);

  currentEvent = calendarManager.begin();
}

Timeline::~Timeline() {
  lv_obj_clean(lv_scr_act());
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

void Timeline::set_timeslot_area(lv_area_t* timeslot_area, uint8_t day_index, uint16_t timeslot_index) {
  const lv_coord_t TIMETABLE_X0 = lv_obj_get_x(timetable_draw_area) + INTER_DAY_SPACE;
  const lv_coord_t TIMETABLE_Y0 = lv_obj_get_y(timetable_draw_area) + INTER_DAY_SPACE;

  const lv_coord_t x_offset = TIMETABLE_X0 + DAY_PX_OFFSET * day_index;
  const lv_coord_t y_offset = TIMETABLE_Y0 + timeslot_index * TIMESLOT_PX_HEIGHT;

  timeslot_area->x1 = x_offset;
  timeslot_area->y1 = y_offset;
  timeslot_area->x2 = x_offset + TIMESLOT_PX_WIDTH  - 1;
  timeslot_area->y2 = y_offset + TIMESLOT_PX_HEIGHT - 1;
}

void Timeline::draw_event(uint8_t day_column, uint16_t event_start, uint16_t duration) {
  uint16_t event_end = event_start + duration;

  // Handle day overflow recursively
  if (event_end > MINUTES_PER_DAY) {
    draw_event(day_column+1, 0, event_end - MINUTES_PER_DAY);
    event_end = MINUTES_PER_DAY;
  }

  // If the event starts before or finishes after the middle of a timeslot, shrink/expand to next timeslot
  const lv_coord_t FIRST_TIMESLOT_UNDERFLOW = (event_start % MINUTES_PER_TIMESLOT > MINUTES_PER_TIMESLOT/2) ? 1 : 0;
  const lv_coord_t LAST_TIMESLOT_OVERFLOW   = (event_end % MINUTES_PER_TIMESLOT < MINUTES_PER_TIMESLOT/2) ? 0 : 1;

  // Compute rounded timeslot event_start and end
  const lv_coord_t FIRST_TIMESLOT_START = event_start / MINUTES_PER_TIMESLOT + FIRST_TIMESLOT_UNDERFLOW;
  const lv_coord_t LAST_TIMESLOT_END    = event_end / MINUTES_PER_TIMESLOT + LAST_TIMESLOT_OVERFLOW;

  lv_area_t timeslot_area;
  for(lv_coord_t timeslot=FIRST_TIMESLOT_START; timeslot < LAST_TIMESLOT_END; timeslot++) {
    set_timeslot_area(&timeslot_area, day_column, timeslot);
    lvgl.SetFullRefresh(Components::LittleVgl::FullRefreshDirections::None);
    lvgl.FlushDisplay(&timeslot_area, timeslotBuffer);
  }
}

void Timeline::displayWeek() {
  lv_obj_set_hidden(time_label, true);
  lv_obj_set_hidden(title_label, true);

  lv_obj_set_hidden(timetable_draw_area, false);
  lv_obj_set_hidden(days_container, false);
  lv_obj_set_hidden(hours_container, false);
  
  auto currentDateTime = dateTimeController.CurrentDateTime();
  auto dp = date::floor<date::days>(currentDateTime);
  auto time = date::make_time(currentDateTime-dp);
  auto yearMonthDay = date::year_month_day(dp);
  today_index = date::weekday(yearMonthDay).iso_encoding() - 1;


  // Wait to allow lvgl to first draw the UI
  lv_refr_now(nullptr);
  uint32_t t = lv_tick_get();
  while(lv_tick_elaps(t) < 100);

  std::fill(timeslotBuffer, &timeslotBuffer[TIMESLOT_BUFFER_SIZE], LV_COLOR_RED);

  for (int di=0; di<7; di++) {
    draw_event(di, 60 *  3, 10*di);
    draw_event(di, 60 *  8 + 45 + 60*(di%2), 90);
    draw_event(di, 60 * 11 + 15 + 60*(di%2), 90);
    draw_event(di, 60 * 14 + 45 + 60*(di%2), 90);
    draw_event(di, 60 * 16 + 15 + 60*(di%2), 90);
    draw_event(di, 60 * 19 + 45 + 60*(di%2), 90);
  }

  lv_refr_now(nullptr);
  uint32_t t2 = lv_tick_get();
  while(lv_tick_elaps(t2) < 100);
  std::fill(timeslotBuffer, &timeslotBuffer[TIMESLOT_BUFFER_SIZE], LV_COLOR_BLUE);

  for(auto & it : calendarManager) {
    time_t timestamp = it.timestamp;
    tm *it_time = localtime(&timestamp);
    uint8_t day_index = it_time->tm_wday-1;
    if (day_index < today_index) {
      day_index += 7;
    }
    
    uint8_t column_index = day_index - today_index;
    if (column_index == 0 && timestamp > currentDateTime.time_since_epoch().count()) {
      continue; // Do not show events matching the current day of the week but occurring next week
    }

    draw_event(column_index, it_time->tm_hour*60 + it_time->tm_min, it.duration/60);
  }
}

void Timeline::displayCurrentEvent() {
  lv_obj_set_hidden(timetable_draw_area, true);
  lv_obj_set_hidden(days_container, true);
  lv_obj_set_hidden(hours_container, true);

  lv_obj_set_hidden(title_label, false);
  lv_obj_set_hidden(time_label, false);

  if (currentEvent == calendarManager.end()) {
    lv_label_set_text_static(title_label, "TIMELINE");
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
