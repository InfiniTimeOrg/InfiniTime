#include "displayapp/screens/TrackTime.h"
#include "displayapp/DisplayApp.h"
#include <lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;

TrackTime::TrackTime(DisplayApp* app) : Screen(app) {
 
  btn1 = lv_btn_create(lv_scr_act(), nullptr);
  //btnPlayPause->user_data = this;
  //lv_obj_set_event_cb(btnPlayPause, play_pause_event_handler);
  lv_obj_set_size(btn1, 115, 50);
  lv_obj_align(btn1, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, -3, -75);
  //txtPlayPause = lv_label_create(btnPlayPause, nullptr);
  //lv_label_set_text_static(txtPlayPause, Symbols::play);

  btn2 = lv_btn_create(lv_scr_act(), nullptr);
  lv_obj_set_size(btn2, 115, 50);
  lv_obj_align(btn2, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 3, -75);


  btn3 = lv_btn_create(lv_scr_act(), nullptr);
  lv_obj_set_size(btn3, 115, 50);
  lv_obj_align(btn3, lv_scr_act(), LV_ALIGN_IN_RIGHT_MID, -3, -35);

  btn4 = lv_btn_create(lv_scr_act(), nullptr);
  lv_obj_set_size(btn4, 115, 50);
  lv_obj_align(btn4, lv_scr_act(), LV_ALIGN_IN_LEFT_MID, 3, -35);
 
  btn5 = lv_btn_create(lv_scr_act(), nullptr);
  lv_obj_set_size(btn5, 115, 50);
  lv_obj_align(btn5, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, -3, 5);

  btn6 = lv_btn_create(lv_scr_act(), nullptr);
  lv_obj_set_size(btn6, 115, 50);
  lv_obj_align(btn6, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 3, 5);

  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Current: ");
  lv_label_set_align(title, LV_LABEL_ALIGN_LEFT);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, -20);
}

TrackTime::~TrackTime() {
  lv_obj_clean(lv_scr_act());
}