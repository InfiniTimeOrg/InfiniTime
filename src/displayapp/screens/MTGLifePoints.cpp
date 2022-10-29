#include "displayapp/screens/MTGLifePoints.h"
#include "displayapp/DisplayApp.h"

using namespace Pinetime::Applications::Screens;

MTGLifePoints::MTGLifePoints(DisplayApp* app) : Screen(app) {
  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Life Points:");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);

  twoDigitCounter.Create();
  lv_obj_align(twoDigitCounter.GetObject(), nullptr, LV_ALIGN_CENTER, 0, 0);
}

MTGLifePoints::~MTGLifePoints() {
  lv_obj_clean(lv_scr_act());
}
