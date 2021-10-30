#include "PassKey.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

PassKey::PassKey(Pinetime::Applications::DisplayApp* app, uint32_t key) : Screen(app) {
  lpasskey = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(lpasskey, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFF00));
  lv_obj_set_style_local_text_font(lpasskey, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_42);
  lv_label_set_text_fmt(lpasskey, "%06u", key);
  lv_obj_align(lpasskey, nullptr, LV_ALIGN_CENTER, 0, -20);
}

PassKey::~PassKey() {
  lv_obj_clean(lv_scr_act());
}

