#include "Label.h"

using namespace Pinetime::Applications::Screens;

Label::Label(Pinetime::Applications::DisplayApp *app, const char *text)  : Screen(app), text{text} {
  label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
  lv_obj_set_size(label, 240, 240);
  lv_label_set_text(label, text);
}

Label::~Label() {
  lv_obj_clean(lv_scr_act());
}
