#include "displayapp/widgets/DotIndicator.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Widgets;

DotIndicator::DotIndicator(uint8_t nCurrentScreen, uint8_t nScreens) : nCurrentScreen {nCurrentScreen}, nScreens {nScreens} {
}

void DotIndicator::Create() {

  uint16_t placer = LV_VER_RES / 2 - nScreens * 8;
  for (int i = 0; i < nScreens; i++) {

    dotIndicator[i] = lv_obj_create(lv_scr_act(), nullptr);
    lv_obj_set_size(dotIndicator[i], 16, 16);
    lv_obj_set_style_local_bg_color(dotIndicator[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_obj_set_pos(dotIndicator[i], LV_HOR_RES - 16, placer);
    placer = placer + 32;
  }

  lv_obj_set_style_local_bg_color(dotIndicator[nCurrentScreen], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
}

