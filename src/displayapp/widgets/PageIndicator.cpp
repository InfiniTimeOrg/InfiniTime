#include "displayapp/widgets/PageIndicator.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Widgets;

PageIndicator::PageIndicator(uint8_t nCurrentScreen, uint8_t nScreens, bool horizontal)
    : nCurrentScreen {nCurrentScreen}, nScreens {nScreens}, horizontal {horizontal} {
}

void PageIndicator::Create() {
  pageIndicatorBasePoints[0].x = horizontal ? 0 : LV_HOR_RES - 1;
  pageIndicatorBasePoints[0].y = horizontal ? LV_VER_RES - 1 : 0;
  pageIndicatorBasePoints[1].x = horizontal ? LV_HOR_RES : LV_HOR_RES - 1;
  pageIndicatorBasePoints[1].y = horizontal ? LV_VER_RES - 1 : LV_VER_RES;

  pageIndicatorBase = lv_line_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_line_width(pageIndicatorBase, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 3);
  lv_obj_set_style_local_line_color(pageIndicatorBase, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, Colors::bgDark);
  lv_line_set_points(pageIndicatorBase, pageIndicatorBasePoints, 2);

  const int16_t indicatorSize = (horizontal ? LV_HOR_RES : LV_VER_RES) / nScreens;
  const int16_t indicatorPos = indicatorSize * nCurrentScreen;

  pageIndicatorPoints[0].x = horizontal ? indicatorPos : LV_HOR_RES - 1;
  pageIndicatorPoints[0].y = horizontal ? LV_VER_RES - 1 : indicatorPos;
  pageIndicatorPoints[1].x = horizontal ? indicatorPos + indicatorSize : LV_HOR_RES - 1;
  pageIndicatorPoints[1].y = horizontal ? LV_VER_RES - 1 : indicatorPos + indicatorSize;

  pageIndicator = lv_line_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_line_width(pageIndicator, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 3);
  lv_obj_set_style_local_line_color(pageIndicator, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_line_set_points(pageIndicator, pageIndicatorPoints, 2);
}
