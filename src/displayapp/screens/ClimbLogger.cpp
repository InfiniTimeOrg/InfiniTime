#include "displayapp/screens/ClimbLogger.h"

using namespace Pinetime::Applications::Screens;

ClimbLogger::ClimbLogger() {
  label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(label, "ClimbLogger");
  lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
}

ClimbLogger::~ClimbLogger() {
  lv_obj_clean(lv_scr_act());
}
