#include "PassKey.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

PassKey::PassKey(Pinetime::Applications::DisplayApp* app, uint32_t key) : Screen(app) {
  passkeyLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(passkeyLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFF00));
  lv_obj_set_style_local_text_font(passkeyLabel, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_fmt(passkeyLabel, "%06u", key);
  lv_obj_align(passkeyLabel, nullptr, LV_ALIGN_CENTER, 0, -20);

  backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_click(backgroundLabel, true);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text(backgroundLabel, "");
}

PassKey::~PassKey() {
  lv_obj_clean(lv_scr_act());
}

