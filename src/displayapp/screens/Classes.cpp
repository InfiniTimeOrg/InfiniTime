//#include "displayapp/screens/Motion.h"
#include "displayapp/screens/Classes.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

Classes::Classes(Controllers::DateTime& dateTimeController) {


  label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(label, "X #FF0000 %d# Y #00B000 %d# Z #FFFF00 %d#", 0, 0, 0);
  lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label, nullptr, LV_ALIGN_IN_TOP_MID, 0, 10);
  lv_label_set_recolor(label, true);

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

Classes::~Classes() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Classes::Refresh() {
  lv_label_set_text_fmt(label, '#ff0fff hello chat!');
}