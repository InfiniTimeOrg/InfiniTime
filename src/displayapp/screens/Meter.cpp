#include "Meter.h"
#include <lvgl/lvgl.h>
#include "../DisplayApp.h"

using namespace Pinetime::Applications::Screens;

Meter::Meter(Pinetime::Applications::DisplayApp* app) : Screen(app) {

  /*Create a line meter */
  lmeter = lv_meter_create(lv_scr_act());
  lscale = lv_meter_add_scale(lmeter);
  lindicator = lv_meter_add_needle_line(lmeter, lscale, 2, lv_color_white(), 0);
  lv_meter_set_scale_ticks(lmeter, lscale, 60, 2, 5, lv_color_white());
  lv_meter_set_scale_range(lmeter, lscale, 0, 60, 360, 180); /*Set the range*/
  lv_meter_set_indicator_value(lmeter, lindicator, value); /*Set the current value*/
  
  lv_meter_add_scale_lines(lmeter, lscale, lv_palette_lighten(LV_PALETTE_GREY, 1), lv_color_make(160, 0, 0), true, 0);

  lv_obj_set_size(lmeter, 200, 200);
  lv_obj_align(lmeter,  LV_ALIGN_CENTER, 0, 0);

  taskRefresh = lv_timer_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, this);
}

Meter::~Meter() {
  lv_timer_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Meter::Refresh() {
  lv_meter_set_indicator_value(lmeter, lindicator, value++); /*Set the current value*/
  if (value >= 60)
    value = 0;
}
