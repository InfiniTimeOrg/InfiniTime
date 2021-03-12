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
      auto id = calendarManager.getCount();
      Controllers::CalendarManager::CalendarEvent tmpEvent {.id = id, .title = {"title"}};
      bool result = calendarManager.addEvent(tmpEvent);
      sprintf(superMegaStringForTesting, "Added e%d: %d", id, result);
      lv_label_set_text(hello_world_label, superMegaStringForTesting);
    } break;

    case TouchEvents::SwipeDown: {
      auto tmpEvent = *calendarManager.begin();
      bool result = calendarManager.deleteEvent(tmpEvent.id);
      sprintf(superMegaStringForTesting, "Deleted e%d: %d", tmpEvent.id, result);
      lv_label_set_text(hello_world_label, superMegaStringForTesting);
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
          superMegaStringForTesting[offset] = 48+(*tmpEvent).id;
          superMegaStringForTesting[offset+1] = ',';
          offset += 2;
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
