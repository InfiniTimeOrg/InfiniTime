#include "List.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static void ButtonEventHandler(lv_obj_t* obj, lv_event_t event) {
    List* screen = static_cast<List*>(obj->user_data);
    screen->OnButtonEvent(obj, event);
  }
  
}

List::List(Pinetime::Applications::DisplayApp* app, uint8_t screenID, uint8_t numScreens)
    : Screen(app) {
  // Set the background to Black
  lv_obj_set_style_local_bg_color(lv_scr_act(), LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, lv_color_make(0, 0, 0));
  //draw scroll bar if List has more than one Page
  if (numScreens > 1) {
    pageIndicatorBasePoints[0].x = 240 - 1;
    pageIndicatorBasePoints[0].y = 6;
    pageIndicatorBasePoints[1].x = 240 - 1;
    pageIndicatorBasePoints[1].y = 240 - 6;
    
    pageIndicatorBase = lv_line_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_line_width(pageIndicatorBase, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_line_color(pageIndicatorBase, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x111111));
    lv_obj_set_style_local_line_rounded(pageIndicatorBase, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, true);
    lv_line_set_points(pageIndicatorBase, pageIndicatorBasePoints, 2);
    
    uint16_t indicatorSize = 228 / numScreens;
    uint16_t indicatorPos = indicatorSize * screenID;
    
    pageIndicatorPoints[0].x = 240 - 1;
    pageIndicatorPoints[0].y = 6 + indicatorPos;
    pageIndicatorPoints[1].x = 240 - 1;
    pageIndicatorPoints[1].y = 6 + indicatorPos + indicatorSize;
    
    pageIndicator = lv_line_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_line_width(pageIndicator, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, 3);
    lv_obj_set_style_local_line_color(pageIndicator, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    lv_obj_set_style_local_line_rounded(pageIndicator, LV_LINE_PART_MAIN, LV_STATE_DEFAULT, true);
    lv_line_set_points(pageIndicator, pageIndicatorPoints, 2);
  }
  
  //container for the buttons that will be created by createButtonNr
  container1 = lv_cont_create(lv_scr_act(), nullptr);
  
  // lv_obj_set_style_local_bg_color(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x111111));
  lv_obj_set_style_local_bg_opa(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_TRANSP);
  lv_obj_set_style_local_pad_all(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 10);
  lv_obj_set_style_local_pad_inner(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 5);
  lv_obj_set_style_local_border_width(container1, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  
  lv_obj_set_pos(container1, 0, 0);
  lv_obj_set_width(container1, LV_HOR_RES - 15);
  lv_obj_set_height(container1, LV_VER_RES);
  lv_cont_set_layout(container1, LV_LAYOUT_COLUMN_LEFT);
  
  lv_obj_t* backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, LV_HOR_RES, LV_VER_RES);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text_static(backgroundLabel, "");
}

//Derived Classes shall call this in their constructor to create the buttons
void List::createButtonNr(int i, Item& item) {
  
  lv_obj_t* labelBt;
  lv_obj_t* labelBtIco;
  
  buttons[i] = lv_btn_create(container1, nullptr);
  lv_obj_set_style_local_bg_opa(buttons[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_OPA_20);
  lv_obj_set_style_local_radius(buttons[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, 20);
  lv_obj_set_style_local_bg_color(buttons[i], LV_BTN_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_AQUA);
  
  lv_obj_set_width(buttons[i], LV_HOR_RES - 25);
  lv_obj_set_height(buttons[i], 52);
  lv_obj_set_event_cb(buttons[i], ButtonEventHandler);
  lv_btn_set_layout(buttons[i], LV_LAYOUT_ROW_MID);
  buttons[i]->user_data = this;
  
  labelBtIco = lv_label_create(buttons[i], nullptr);
  lv_obj_set_style_local_text_color(labelBtIco, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  lv_label_set_text_static(labelBtIco, item.icon);
  
  labelBt = lv_label_create(buttons[i], nullptr);
  lv_label_set_text_fmt(labelBt, " %s", item.name);
}
