#include "displayapp/screens/TapCounter.h"

using namespace Pinetime::Applications::Screens;

namespace {
  // LVGL is a C library, so events arrive via a plain function pointer.
  // The standard InfiniTime pattern: stash `this` in the widget's
  // user_data, then bounce the event back into a member function.
  void ButtonEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<TapCounter*>(obj->user_data);
    if (event == LV_EVENT_CLICKED) {
      screen->OnButtonClicked();
    }
  }
}

TapCounter::TapCounter() {
  // The constructor builds the UI. lv_scr_act() is the active screen;
  // every widget is parented to it (or to another widget).

  counterLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(counterLabel, "Count: %d", count);
  lv_label_set_align(counterLabel, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(counterLabel, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 60);

  button = lv_btn_create(lv_scr_act(), nullptr);
  button->user_data = this;
  lv_obj_set_event_cb(button, ButtonEventHandler);
  lv_obj_set_size(button, 200, 80);
  lv_obj_align(button, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, -20);

  buttonLabel = lv_label_create(button, nullptr);
  lv_label_set_text_static(buttonLabel, "Tap me");
}

TapCounter::~TapCounter() {
  // Destructor must clean up everything LVGL created, otherwise the
  // widgets would leak into whatever screen is shown next.
  lv_obj_clean(lv_scr_act());
}

void TapCounter::OnButtonClicked() {
  count++;
  lv_label_set_text_fmt(counterLabel, "Count: %d", count);
}
