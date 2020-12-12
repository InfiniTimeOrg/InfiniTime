#include "Meter.h"
#include <lvgl/lvgl.h>
#include "../DisplayApp.h"

using namespace Pinetime::Applications::Screens;
extern lv_font_t jetbrains_mono_extrabold_compressed;
extern lv_font_t jetbrains_mono_bold_20;


Meter::Meter(Pinetime::Applications::DisplayApp *app) : Screen(app) {

  lv_style_copy(&style_lmeter, &lv_style_pretty_color);
  style_lmeter.line.width = 2;
  style_lmeter.line.color = LV_COLOR_SILVER;
  style_lmeter.body.main_color = lv_color_make(255,0,0);
  style_lmeter.body.grad_color = lv_color_make(160,0,0);
  style_lmeter.body.padding.left = 16;                           /*Line length*/

  /*Create a line meter */
  lmeter = lv_lmeter_create(lv_scr_act(), nullptr);
  lv_lmeter_set_range(lmeter, 0, 60);                   /*Set the range*/
  lv_lmeter_set_value(lmeter, value);                       /*Set the current value*/
  lv_lmeter_set_angle_offset(lmeter, 180);
  lv_lmeter_set_scale(lmeter, 360, 60);                  /*Set the angle and number of lines*/
  lv_lmeter_set_style(lmeter, LV_LMETER_STYLE_MAIN, &style_lmeter);           /*Apply the new style*/
  lv_obj_set_size(lmeter, 150, 150);
  lv_obj_align(lmeter, nullptr, LV_ALIGN_CENTER, 0, 0);

}

Meter::~Meter() {


  lv_obj_clean(lv_scr_act());
}

bool Meter::Refresh() {
  lv_lmeter_set_value(lmeter, value++);                       /*Set the current value*/
  if(value>=60) value = 0;

  return running;
}

bool Meter::OnButtonPushed() {
  running = false;
  return true;
}
