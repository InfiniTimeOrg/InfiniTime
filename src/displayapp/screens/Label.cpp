#include "Label.h"

using namespace Pinetime::Applications::Screens;

Label::Label(uint8_t screenID, uint8_t numScreens, Pinetime::Applications::DisplayApp* app, lv_obj_t* labelText)
  : Screen(app), labelText {labelText} {

  if (numScreens > 1) {
    pageIndicatorBasePoints[0].x = 240 - 1;
    pageIndicatorBasePoints[0].y = 6;
    pageIndicatorBasePoints[1].x = 240 - 1;
    pageIndicatorBasePoints[1].y = 240 - 6;

    pageIndicatorBase = lv_line_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_line_width(pageIndicatorBase, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_line_color(pageIndicatorBase, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x111111));
    lv_obj_set_style_local_line_rounded(pageIndicatorBase, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, true);
    lv_line_set_points(pageIndicatorBase, pageIndicatorBasePoints, 2);

    uint16_t indicatorSize = 228 / numScreens;
    uint16_t indicatorPos = indicatorSize * screenID;

    pageIndicatorPoints[0].x = 240 - 1;
    pageIndicatorPoints[0].y = (6 + indicatorPos);
    pageIndicatorPoints[1].x = 240 - 1;
    pageIndicatorPoints[1].y = (6 + indicatorPos) + indicatorSize;

    pageIndicator = lv_line_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_line_width(pageIndicator, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_line_color(pageIndicator, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_obj_set_style_local_line_rounded(pageIndicator, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, true);
    lv_line_set_points(pageIndicator, pageIndicatorPoints, 2);
  }
}

Label::~Label() {
  lv_obj_clean(lv_scr_act());
}
