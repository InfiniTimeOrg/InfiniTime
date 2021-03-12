#include "Timeline.h"

#include <lvgl/lvgl.h>
#include <cstdio>

using namespace Pinetime::Applications::Screens;

char superMegaStringForTesting[100];

Timeline::Timeline(DisplayApp* app, Controllers::CalendarManager& calendarManager)
  : Screen(app), calendarManager(calendarManager) {
  hello_world_label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(hello_world_label, "Time to timeline :-)");
  lv_obj_set_auto_realign(hello_world_label, true);
  lv_obj_align(hello_world_label, nullptr, LV_ALIGN_CENTER, 0, 0);

  for (int i = 0; i < 100; ++i) {
    superMegaStringForTesting[i] = 0;
  }

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
      sprintf(superMegaStringForTesting, "Nb events: %d", nbEvents);
      lv_label_set_text(hello_world_label, superMegaStringForTesting);
    } break;

    case TouchEvents::SwipeRight: {
      if (calendarManager.empty()) {
        strcpy(superMegaStringForTesting, "No event");
      } else {
        strcpy(superMegaStringForTesting, "Evts: ");
        auto tmpEvent = calendarManager.begin();
        int offset = 6;
        while (tmpEvent != calendarManager.end()) {
          offset += sprintf(&superMegaStringForTesting[offset], "%d,", (*tmpEvent).id);
          tmpEvent++;
        }
        superMegaStringForTesting[offset] = '\0';
      }
      lv_label_set_text(hello_world_label, superMegaStringForTesting);
    } break;

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
    lv_label_set_text(hello_world_label, "No event");
  } else {
    auto event = *currentEvent;
    sprintf(superMegaStringForTesting, "e%d: %s", event.id, event.title.c_str());
    lv_label_set_text(hello_world_label, superMegaStringForTesting);
  }
}
