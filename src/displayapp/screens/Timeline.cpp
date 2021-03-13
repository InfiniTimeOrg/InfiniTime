#include "Timeline.h"

#include <ctime>
#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;

void formatDateTime(char* buffer, time_t timestamp) {
  auto *time = gmtime(&timestamp);
  auto year = 1900 + time->tm_year;
  auto month = 1 + time->tm_mon;
  auto day = time->tm_mday;
  auto hour = time->tm_hour;
  auto minute = time->tm_min;
  sprintf(buffer, "%d-%d-%d %d:%d", year, month, day, hour, minute);
}

Timeline::Timeline(DisplayApp* app, Controllers::CalendarManager& calendarManager)
  : Screen(app), calendarManager(calendarManager) {
  title_label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(title_label, "TIMELINE");
  lv_obj_set_auto_realign(title_label, true);
  lv_obj_align(title_label, nullptr, LV_ALIGN_IN_TOP_MID, 0, 0);

  time_label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(time_label, "Swipe right\nfor help");
  lv_obj_set_auto_realign(time_label, true);
  lv_obj_align(time_label, nullptr, LV_ALIGN_CENTER, 0, 0);

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
    case TouchEvents::SwipeUp: {
      auto calBegin = calendarManager.begin();
      if (currentEvent == calendarManager.end()) {
        currentEvent = calBegin;
      } else {
        currentEvent--;
      }

      displayCurrent();
    } break;

    case TouchEvents::SwipeDown: {
      auto calBegin = calendarManager.begin();
      if (currentEvent == calendarManager.end()) {
        currentEvent = calBegin;
      } else {
        currentEvent++;
      }

      displayCurrent();
    } break;

    case TouchEvents::SwipeLeft: {
      auto nbEvents = calendarManager.getCount();
      lv_label_set_text(title_label, "TIMELINE - Stats");
      lv_label_set_text_fmt(time_label, "Nb events: %d", nbEvents);
    } break;

    case TouchEvents::SwipeRight: {
      lv_label_set_text(title_label, "TIMELINE - Help");
      lv_label_set_text(time_label, "Swipe to use\nUp/Down: prev/next\nLeft: stats\nRight: help");
    }
    default:
      return false;
  }
  return true;
}

bool Timeline::OnTouchEvent(uint16_t x, uint16_t y) {
  return Screen::OnTouchEvent(x, y);
}

void Timeline::displayCurrent() {
  if (currentEvent == calendarManager.end()) {
    lv_label_set_text(title_label, "");
    lv_label_set_text(time_label, "No event");
  } else {
    auto event = *currentEvent;
    lv_label_set_text_fmt(title_label, "%s", event.title.c_str());

    char begin[16], end[16];
    formatDateTime(begin, event.timestamp);
    formatDateTime(end, event.timestamp + event.duration);
    lv_label_set_text_fmt(time_label, "%s\n->\n%s", begin, end);
  }
}
