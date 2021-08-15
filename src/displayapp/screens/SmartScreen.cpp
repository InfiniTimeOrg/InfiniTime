#include "SmartScreen.h"

#include <lvgl/lvgl.h>
#include <cstdio>
#include "Symbols.h"

using namespace Pinetime::Applications::Screens;

SmartScreen::SmartScreen(DisplayApp* app)
  : Screen(app) {

  
  bButton1State = false;
  bButton2State = false;
  bButton3State = false;

  labelTitle = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(labelTitle, "SmartControl");
  lv_obj_align(labelTitle, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);

  labelValue1 = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(labelValue1, "Temp = 25.5 C");
  lv_obj_align(labelValue1, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 30);
                    
  labelValue2 = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(labelValue2, "Humi = 47.3 %");
  lv_obj_align(labelValue2, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 50);

  labelValue3 = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(labelValue3, "Batt = 98 %");
  lv_obj_align(labelValue3, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 70);             
                    


  button1 = lv_btn_create(lv_scr_act(), nullptr);
  button1->user_data = this;
  lv_obj_set_event_cb(button1, btn_event_cb);
  lv_obj_set_size(button1, 76, 76);
  lv_obj_align(button1, NULL, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  textButton1 = lv_label_create(button1, nullptr);
  lv_label_set_text(textButton1, "ON");
  lv_obj_set_style_local_bg_color(button1, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLUE);

  button2 = lv_btn_create(lv_scr_act(), nullptr);
  button2->user_data = this;
  lv_obj_set_event_cb(button2, btn_event_cb);
  lv_obj_set_size(button2, 76, 76);
  lv_obj_align(button2, NULL, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  textButton2 = lv_label_create(button2, nullptr);
  lv_label_set_text(textButton2, "ON");
  lv_obj_set_style_local_bg_color(button2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLUE);

  button3 = lv_btn_create(lv_scr_act(), nullptr);
  button3->user_data = this;
  lv_obj_set_event_cb(button3, btn_event_cb);
  lv_obj_set_size(button3, 76, 76);
  lv_obj_align(button3, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  textButton3 = lv_label_create(button3, nullptr);
  lv_label_set_text(textButton3, "ON");
  lv_obj_set_style_local_bg_color(button3, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLUE);

  labelButton1 = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(labelButton1, "BTN1");
  lv_obj_align(labelButton1, button1, LV_ALIGN_OUT_TOP_MID, 0, 0);             
                    
  labelButton2 = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(labelButton2, "BTN2");
  lv_obj_align(labelButton2, button2, LV_ALIGN_OUT_TOP_MID, 0, 0);    

  labelButton3 = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(labelButton3, "BTN3");
  lv_obj_align(labelButton3, button3, LV_ALIGN_OUT_TOP_MID, 0, 0);    
 
  backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_click(backgroundLabel, true);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text(backgroundLabel, "");
}

SmartScreen::~SmartScreen() {
  lv_obj_clean(lv_scr_act());
}

bool SmartScreen::Refresh()
{
    return true;
}


void SmartScreen::btn_event_cb(lv_obj_t * btn, lv_event_t event)
{
    // When button 1 gets pressed, call this function
    // And send the data to companion app
    // Check the button state, if is activated, send deactivate command
    //   if it is not active, send activate command
}

void SmartScreen::UpdateValues()
{

}