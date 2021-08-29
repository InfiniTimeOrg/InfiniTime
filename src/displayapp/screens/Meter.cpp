#include "Meter.h"
#include <lvgl/lvgl.h>
#include "../DisplayApp.h"

using namespace Pinetime::Applications::Screens;

Meter::Meter(Pinetime::Applications::DisplayApp* app) : Screen(app) {

  /*Create a line meter */
  lmeter = lv_linemeter_create(lv_scr_act(), nullptr);
  lv_linemeter_set_range(lmeter, 0, 60); /*Set the range*/
  lv_linemeter_set_value(lmeter, value); /*Set the current value*/
  lv_linemeter_set_angle_offset(lmeter, 180);
  lv_linemeter_set_scale(lmeter, 360, 60); /*Set the angle and number of lines*/

  lv_obj_set_style_local_scale_end_color(lmeter, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(255, 0, 0));
  lv_obj_set_style_local_scale_grad_color(lmeter, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(160, 0, 0));
  lv_obj_set_style_local_line_width(lmeter, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, 2);
  lv_obj_set_style_local_line_color(lmeter, LV_LINEMETER_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_SILVER);

  lv_obj_set_size(lmeter, 200, 200);
  lv_obj_align(lmeter, nullptr, LV_ALIGN_CENTER, 0, 0);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

Meter::~Meter() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Meter::Refresh() {
  lv_linemeter_set_value(lmeter, value++); /*Set the current value*/
  if (value >= 60)
    value = 0;
}
