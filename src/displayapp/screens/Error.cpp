#include "Error.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void ButtonEventCallback(lv_event_t* event) {
    auto* errorScreen = static_cast<Error*>(lv_event_get_user_data(event));
    errorScreen->ButtonEventHandler();
  }
}

Error::Error(Pinetime::Applications::DisplayApp* app, System::BootErrors error)
  : Screen(app) {

  lv_obj_t* warningLabel = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(warningLabel, lv_palette_main(LV_PALETTE_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text_static(warningLabel, "Warning");
  lv_obj_align(warningLabel, LV_ALIGN_TOP_MID, 0, 0);

  lv_obj_t* causeLabel = lv_label_create(lv_scr_act());
  lv_label_set_long_mode(causeLabel, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(causeLabel, LV_HOR_RES);
  lv_obj_align_to(causeLabel, warningLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

  if (error == System::BootErrors::TouchController) {
    lv_label_set_text_static(causeLabel, "Touch controller error detected.");
  }

  lv_obj_t* tipLabel = lv_label_create(lv_scr_act());
  lv_label_set_long_mode(tipLabel, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(tipLabel, LV_HOR_RES);
  lv_label_set_text_static(tipLabel, "If you encounter problems and your device is under warranty, contact the devices seller.");
  lv_obj_align_to(tipLabel, causeLabel, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

  btnOk = lv_btn_create(lv_scr_act());
  btnOk->user_data = this;
  lv_obj_add_event_cb(btnOk, ButtonEventCallback, LV_EVENT_ALL, btnOk->user_data);
  lv_obj_set_size(btnOk, LV_HOR_RES, 50);
  lv_obj_align(btnOk, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_style_bg_color(btnOk, lv_palette_main(LV_PALETTE_ORANGE), LV_PART_MAIN | LV_STATE_DEFAULT);
  lblOk = lv_label_create(btnOk);
  lv_obj_center(lblOk);
  lv_label_set_text(btnOk, "Proceed");
}

void Error::ButtonEventHandler() {
  running = false;
}

Error::~Error() {
  lv_obj_clean(lv_scr_act());
}
