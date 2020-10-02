#include <libs/lvgl/lvgl.h>
#include "Gauge.h"
#include "../DisplayApp.h"

using namespace Pinetime::Applications::Screens;
extern lv_font_t jetbrains_mono_extrabold_compressed;
extern lv_font_t jetbrains_mono_bold_20;


Gauge::Gauge(Pinetime::Applications::DisplayApp *app) : Screen(app) {
  /*Create a style*/
  lv_style_copy(&style, &lv_style_pretty_color);
  style.body.main_color = LV_COLOR_CYAN;     /*Line color at the beginning*/
  style.body.grad_color =  LV_COLOR_RED;    /*Line color at the end*/
  style.body.padding.left = 10;                      /*Scale line length*/
  style.body.padding.inner = 8 ;                    /*Scale label padding*/
  style.body.border.color = lv_color_hex3(0x333);   /*Needle middle circle color*/
  style.line.width = 3;
  style.text.color = LV_COLOR_WHITE;
  style.line.color = LV_COLOR_RED;                  /*Line color after the critical value*/


  /*Describe the color for the needles*/

  needle_colors[0] = LV_COLOR_ORANGE;

  /*Create a gauge*/
  gauge1 = lv_gauge_create(lv_scr_act(), NULL);
  lv_gauge_set_style(gauge1, LV_GAUGE_STYLE_MAIN, &style);
  lv_gauge_set_needle_count(gauge1, 1, needle_colors);
  lv_obj_set_size(gauge1, 180, 180);
  lv_obj_align(gauge1, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_gauge_set_scale(gauge1, 360, 60, 0);
  lv_gauge_set_range(gauge1, 0, 59);

  /*Set the values*/
  lv_gauge_set_value(gauge1, 0, value);
}

Gauge::~Gauge() {


  lv_obj_clean(lv_scr_act());
}

bool Gauge::Refresh() {
//  lv_lmeter_set_value(lmeter, value++);                       /*Set the current value*/
//  if(value>=60) value = 0;

  lv_gauge_set_value(gauge1, 0, value++);
  if(value == 59) value = 0;
  return running;
}

bool Gauge::OnButtonPushed() {
  running = false;
  return true;
}
