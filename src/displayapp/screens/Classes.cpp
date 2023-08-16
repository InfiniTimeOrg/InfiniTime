//#include "displayapp/screens/Motion.h"
#include "displayapp/screens/Classes.h"
#include <lvgl/lvgl.h>
#include "displayapp/DisplayApp.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

//Classes::Classes(Controllers::DateTime& dateTimeController) {

//  label
//  next_class_start
//  next_class_end
//  next_class_name
//  next_class_loc

Classes::Classes() {

  label = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(label, "Next Class:", 0, 0, 0);
  lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(label, nullptr, LV_ALIGN_IN_TOP_MID, 0, 10);
  lv_label_set_recolor(label, true);

  next_class_name = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(next_class_name, "Loading Next Class", 0, 0, 0);
  lv_label_set_align(next_class_name, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(next_class_name, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
  lv_label_set_recolor(next_class_name, true);

  next_class_loc = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(next_class_loc, "Loading Class Loc", 0, 0, 0);
  lv_label_set_align(next_class_loc, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(next_class_loc, next_class_name, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
  lv_label_set_recolor(next_class_loc, true);

  next_class_start = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(next_class_start, "Loading Class Start", 0, 0, 0);
  lv_label_set_align(next_class_start, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(next_class_start, next_class_loc, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
  lv_label_set_recolor(next_class_start, true);

  next_class_end = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_fmt(next_class_end, "Loading Class End", 0, 0, 0);
  lv_label_set_align(next_class_end, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(next_class_end, next_class_start, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
  lv_label_set_recolor(next_class_end, true);

  num_list[ "MWF"] = 1;
  num_list[   "W"] = 2;
  num_list[  "TR"] = 3;
  num_list[  "TR"] = 4;
  num_list[ "MWF"] = 5;
  num_list[  "TR"] = 6;

  start_list[1] = "10:00 AM";
  start_list[2] = "12:00 PM";
  start_list[3] =  "2:40 PM";
  start_list[4] = "10:50 AM";
  start_list[5] =  "9:00 AM";
  start_list[6] =  "2:25 PM";
  
  end_list[1]   = "10:50 AM";
  end_list[2]   =  "1:50 PM";
  end_list[3]   =  "3:55 PM";
  end_list[4]   = "12:05 PM";
  end_list[5]   =  "9:50 AM";
  end_list[6]   = "10:40 AM";
  
  name_list[1]  = "Comp Sci 2, 339";
  name_list[2]  = "Comp Lab 2, 329";
  name_list[3]  = "Film Appri, 108";
  name_list[4]  = "Health&Wel, 232";
  name_list[5]  = "Colg Algra, 212";
  name_list[6]  = "Colg Write, 202";
   
  loc_list[1]   = "Math CSci & Tek";
  loc_list[2]   = "Math CSci & Tek";
  loc_list[3]   = "Stanly Russ Hal";
  loc_list[4]   = "Doyne Heath Cnt";
  loc_list[5]   = "Math CSci & Tek";
  loc_list[6]   = "Win Thompson Hl";

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

Classes::~Classes() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void Classes::Refresh() {
  //lv_label_set_text_fmt(next_class, "#ff0fff hello chat!");

  //lv_label_set_text_fmt(label,            "%s", start_list[1]);
  lv_label_set_text_fmt(next_class_name,  "%s",  name_list[1]);
  lv_label_set_text_fmt(next_class_loc,   "%s",   loc_list[1]);
  lv_label_set_text_fmt(next_class_start, "%s", start_list[1]);
  lv_label_set_text_fmt(next_class_end,   "%s",   end_list[1]);

}