#include "displayapp/screens/LucidDreamer.h"
#include "displayapp/DisplayApp.h"
#include "displayapp/screens/Symbols.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void eventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<LucidDreamer*>(obj->user_data);
    screen->OnEvent(obj, event);
  }
}

LucidDreamer::LucidDreamer(DisplayApp* app,
                           Pinetime::Controllers::MotorController& motor)
  : Screen(app), motor {motor} {
  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "lucid\ndreamer");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

  play1 = lv_btn_create(lv_scr_act(), nullptr);
  play1->user_data = this;
  lv_obj_set_event_cb(play1, eventHandler);
  lv_obj_set_size(play1, 75, 50);
  lv_obj_align(play1, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  lblplay1 = lv_label_create(play1, nullptr);
  lv_label_set_text_static(lblplay1, Symbols::play);

  play2 = lv_btn_create(lv_scr_act(), nullptr);
  play2->user_data = this;
  lv_obj_set_event_cb(play2, eventHandler);
  lv_obj_set_size(play2, 75, 50);
  lv_obj_align(play2, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  lblplay2 = lv_label_create(play2, nullptr);
  lv_label_set_text_static(lblplay2, Symbols::play);

  play3 = lv_btn_create(lv_scr_act(), nullptr);
  play3->user_data = this;
  lv_obj_set_event_cb(play3, eventHandler);
  lv_obj_set_size(play3, 75, 50);
  lv_obj_align(play3, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  lblplay3 = lv_label_create(play3, nullptr);
  lv_label_set_text_static(lblplay3, Symbols::play);

}

LucidDreamer::~LucidDreamer() {
  lv_obj_clean(lv_scr_act());
}

void LucidDreamer::OnEvent(lv_obj_t* obj, lv_event_t event) {
  switch (event) {
    case LV_EVENT_CLICKED: {
      if (obj == play1) {
        motor.RunForDuration(50);
      }
      if (obj == play2) {
        motor.RunTest();
      }
      if (obj == play3) {
        motor.VibrateTune();
      }
      break;
    }
    default:
      break;
  }
}
