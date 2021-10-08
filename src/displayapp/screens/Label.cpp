#include "Label.h"

using namespace Pinetime::Applications::Screens;

Label::Label(uint8_t screenID, uint8_t numScreens, Pinetime::Applications::DisplayApp* app, lv_obj_t* labelText)
  : Screen(app), labelText {labelText} {

  if (numScreens > 1) {
    pageIndicatorBasePoints[0].x = LV_HOR_RES - 1;
    pageIndicatorBasePoints[0].y = 0;
    pageIndicatorBasePoints[1].x = LV_HOR_RES - 1;
    pageIndicatorBasePoints[1].y = LV_VER_RES;

    pageIndicatorBase = lv_line_create(lv_scr_act());
    lv_obj_set_style_line_width(pageIndicatorBase, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(pageIndicatorBase, lv_color_hex(0x111111), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_rounded(pageIndicatorBase, true, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_line_set_points(pageIndicatorBase, pageIndicatorBasePoints, 2);

    uint16_t indicatorSize = LV_VER_RES / numScreens;
    uint16_t indicatorPos = indicatorSize * screenID;

    pageIndicatorPoints[0].x = LV_HOR_RES - 1;
    pageIndicatorPoints[0].y = indicatorPos;
    pageIndicatorPoints[1].x = LV_HOR_RES - 1;
    pageIndicatorPoints[1].y = indicatorPos + indicatorSize;

    pageIndicator = lv_line_create(lv_scr_act());
    lv_obj_set_style_line_width(pageIndicator, 3, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(pageIndicator, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_line_rounded(pageIndicator, true, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_line_set_points(pageIndicator, pageIndicatorPoints, 2);
  }
}

Label::~Label() {
  lv_obj_clean(lv_scr_act());
}
