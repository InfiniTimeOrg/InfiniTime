#include "displayapp/widgets/DotIndicator.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Widgets;

DotIndicator::DotIndicator(uint8_t nCurrentScreen, uint8_t nScreens) : nCurrentScreen {nCurrentScreen}, nScreens {nScreens} {
}

void DotIndicator::Create() {
  lv_obj_t* dotIndicator[nScreens];
  static constexpr uint8_t dotSize = 12;

  lv_obj_t* container = lv_cont_create(lv_scr_act(), nullptr);
  lv_cont_set_layout(container, LV_LAYOUT_COLUMN_LEFT);
  lv_cont_set_fit(container, LV_FIT_TIGHT);
  lv_obj_set_style_local_pad_inner(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, dotSize);
  lv_obj_set_style_local_bg_opa(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);

  for (int i = 0; i < nScreens; i++) {
    dotIndicator[i] = lv_obj_create(container, nullptr);
    lv_obj_set_size(dotIndicator[i], dotSize, dotSize);
    lv_obj_set_style_local_bg_color(dotIndicator[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
  }

  lv_obj_set_style_local_bg_color(dotIndicator[nCurrentScreen], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);

  lv_obj_align(container, nullptr, LV_ALIGN_IN_RIGHT_MID, 0, 0);
}
