#include <cstdio>
#include <libs/date/includes/date/date.h>
#include <Components/DateTime/DateTimeController.h>
#include <Version.h>
#include <libs/lvgl/src/lv_core/lv_obj.h>
#include <libs/lvgl/lvgl.h>
#include "Message.h"

using namespace Pinetime::Applications::Screens;

lv_obj_t * label;
void Message::Refresh(bool fullRefresh) {
  if(fullRefresh) {
    lv_obj_t * btn = lv_btn_create(lv_scr_act(), NULL);     /*Add a button the current screen*/
    lv_obj_set_pos(btn, 10, 10);                            /*Set its position*/
    lv_obj_set_size(btn, 100, 50);                          /*Set its size*/
    label = lv_label_create(btn, NULL);          /*Add a label to the button*/
    lv_label_set_text(label, "Button");                     /*Set the labels text*/
  }

}
