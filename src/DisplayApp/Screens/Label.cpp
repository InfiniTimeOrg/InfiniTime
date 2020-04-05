#include <libs/lvgl/lvgl.h>
#include "Label.h"

using namespace Pinetime::Applications::Screens;


Label::Label(const char* text) : text{text} {

}

Label::~Label() {

}

void Label::Refresh() {

}

void Label::Show() {
  label = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_align(label, LV_LABEL_ALIGN_LEFT);
  lv_obj_set_size(label, 240, 240);
  lv_label_set_text(label, text);
}

void Label::Hide() {
  lv_obj_clean(lv_scr_act());
}
