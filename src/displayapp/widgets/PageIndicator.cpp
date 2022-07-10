#include "displayapp/widgets/PageIndicator.h"

using namespace Pinetime::Applications::Widgets;

PageIndicator::PageIndicator(uint8_t nCurrentScreen, uint8_t nScreens) : nCurrentScreen {nCurrentScreen}, nScreens {nScreens} {
}

void PageIndicator::Create() {
  pageIndicatorBasePoints[0].x = LV_HOR_RES - 1;
  pageIndicatorBasePoints[0].y = 0;
  pageIndicatorBasePoints[1].x = LV_HOR_RES - 1;
  pageIndicatorBasePoints[1].y = LV_VER_RES;

  pageIndicatorBase = lv_line_create(lv_scr_act(), nullptr);
  lv_obj_set_state(pageIndicatorBase, LV_STATE_DISABLED);
  lv_line_set_points(pageIndicatorBase, pageIndicatorBasePoints, 2);

  const int16_t indicatorSize = LV_VER_RES / nScreens;
  const int16_t indicatorPos = indicatorSize * nCurrentScreen;

  pageIndicatorPoints[0].x = LV_HOR_RES - 1;
  pageIndicatorPoints[0].y = indicatorPos;
  pageIndicatorPoints[1].x = LV_HOR_RES - 1;
  pageIndicatorPoints[1].y = indicatorPos + indicatorSize;

  pageIndicator = lv_line_create(lv_scr_act(), nullptr);
  lv_line_set_points(pageIndicator, pageIndicatorPoints, 2);
}
