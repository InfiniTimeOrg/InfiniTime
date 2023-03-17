#include "displayapp/screens/Motion.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

Motion::Motion(Controllers::MotionController& motionController) : motionController {motionController} {
  chart = lv_chart_create(lv_scr_act(), nullptr);
  lv_obj_set_size(chart, 240, 240);
  lv_obj_align(chart, nullptr, LV_ALIGN_IN_TOP_MID, 0, 0);
  lv_chart_set_type(chart, LV_CHART_TYPE_LINE); /*Show lines and points too*/
  // lv_chart_set_series_opa(chart, LV_OPA_70);                            /*Opacity of the data series*/
  // lv_chart_set_series_width(chart, 4);                                  /*Line width and point radious*/

  lv_chart_set_range(chart, -1100, 1100);
  lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_SHIFT);
  lv_chart_set_point_count(chart, 10);

  /*Add 3 data series*/
  ser1 = lv_chart_add_series(chart, LV_COLOR_RED);
  ser2 = lv_chart_add_series(chart, Colors::green);
  ser3 = lv_chart_add_series(chart, LV_COLOR_YELLOW);

  lv_chart_init_points(chart, ser1, 0);
  lv_chart_init_points(chart, ser2, 0);
  lv_chart_init_points(chart, ser3, 0);
  lv_chart_refresh(chart); /*Required after direct set*/

  label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(label, "X #FF0000 %d# Y #00B000 %d# Z #FFFF00 %d#", 0, 0, 0);
  lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label, nullptr, LV_ALIGN_IN_TOP_MID, 0, 10);
  lv_label_set_recolor(label, true);

  labelStep = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_align(labelStep, chart, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  lv_label_set_text_static(labelStep, "Steps ---");

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

Motion::~Motion() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Motion::Refresh() {
  lv_chart_set_next(chart, ser1, motionController.X());
  lv_chart_set_next(chart, ser2, motionController.Y());
  lv_chart_set_next(chart, ser3, motionController.Z());

  lv_label_set_text_fmt(labelStep, "Steps %lu", motionController.NbSteps());

  lv_label_set_text_fmt(label,
                        "X #FF0000 %d# Y #00B000 %d# Z #FFFF00 %d#",
                        motionController.X() / 0x10,
                        motionController.Y() / 0x10,
                        motionController.Z() / 0x10);
  lv_obj_align(label, nullptr, LV_ALIGN_IN_TOP_MID, 0, 10);
}
