#include "Notifications_swscroll.h"
#include "displayapp/DisplayApp.h"
#include <libs/lvgl/lvgl.h>

using namespace Pinetime::Applications::Screens;

Notifications2::Notifications2(DisplayApp* app) : Screen(app) {

  app->SetTouchMode(DisplayApp::TouchModes::Polling);
}

Notifications2::~Notifications2() {
  lv_obj_clean(lv_scr_act());
}

bool Notifications2::Refresh() {
  return running;
}

void Notifications2::OnObjectEvent(lv_obj_t *obj, lv_event_t event, uint32_t buttonId) {

}

bool Notifications2::OnTouchEvent(Pinetime::Applications::TouchEvents event) { return true; }


bool Notifications2::OnButtonPushed() {
  app->StartApp(Apps::Clock);
  running = false;
  return true;
}


Notifications2::ListWidget::ListWidget() {
  static lv_point_t valid_pos[] = {{0,0}, {0,1}};
  page = lv_tileview_create(lv_scr_act(), NULL);
  lv_obj_set_size(page, LV_HOR_RES, LV_VER_RES);
  lv_obj_align(page, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_tileview_set_valid_positions(page, valid_pos, 2);

  static lv_style_t pageStyle;
  lv_style_copy(&pageStyle, lv_tileview_get_style(page, LV_TILEVIEW_STYLE_MAIN));

  lv_tileview_set_style(page, LV_TILEVIEW_STYLE_MAIN, &pageStyle);


  lv_obj_t* container1 = lv_cont_create(page, NULL);
  static lv_style_t contStyle;
  lv_style_copy(&contStyle, lv_cont_get_style(container1, LV_CONT_STYLE_MAIN));
  contStyle.body.padding.inner = 20;
  lv_cont_set_style(container1, LV_CONT_STYLE_MAIN, &contStyle);
  lv_obj_set_width(container1, LV_HOR_RES);
  lv_obj_set_pos(container1, 0, 0);
  lv_cont_set_layout(container1, LV_LAYOUT_COL_M);
  lv_cont_set_fit2(container1, LV_FIT_FLOOD, LV_FIT_TIGHT);

  lv_tileview_add_element(page, container1);


  lv_obj_t* t1 = lv_label_create(container1, NULL);

  static lv_style_t titleStyle;
  static lv_style_t textStyle;
  lv_style_copy(&titleStyle, lv_label_get_style(t1, LV_LABEL_STYLE_MAIN));
  lv_style_copy(&textStyle, lv_label_get_style(t1, LV_LABEL_STYLE_MAIN));

  //titleStyle.body.main_color = LV_COLOR_RED;
  //titleStyle.body.grad_color = LV_COLOR_RED;
  titleStyle.body.padding.inner = 5;

  //textStyle.body.main_color = LV_COLOR_BLUE;
  //textStyle.body.grad_color = LV_COLOR_BLUE;
  textStyle.body.padding.inner = 5;

  lv_label_set_style(t1, LV_LABEL_STYLE_MAIN, &titleStyle);
  lv_label_set_long_mode(t1, LV_LABEL_LONG_BREAK);
  lv_label_set_body_draw(t1, true);
  lv_obj_set_width(t1, LV_HOR_RES - (titleStyle.body.padding.left + titleStyle.body.padding.right) );
  lv_label_set_text(t1, "Message");


  lv_obj_t* l1 = lv_label_create(container1, NULL);
  lv_label_set_style(l1, LV_PAGE_STYLE_BG, &textStyle);
  lv_label_set_long_mode(l1, LV_LABEL_LONG_BREAK);
  lv_label_set_body_draw(l1, true);
  lv_obj_set_width(l1, LV_HOR_RES - (textStyle.body.padding.left + textStyle.body.padding.right) );
  lv_label_set_text(l1, "Marcel Pickett: Did you bring your ticket?");

  /*---*/
  lv_obj_t* container2 = lv_cont_create(page, NULL);
  lv_cont_set_style(container2, LV_CONT_STYLE_MAIN, &contStyle);
  lv_obj_set_width(container2, LV_HOR_RES);
  lv_obj_set_pos(container2, 0, lv_obj_get_y(container1) + lv_obj_get_height(container1)+5);
  lv_cont_set_layout(container2, LV_LAYOUT_COL_M);
  lv_cont_set_fit2(container2, LV_FIT_FLOOD, LV_FIT_TIGHT);
  lv_tileview_add_element(page, container2);

  lv_obj_t* t2 = lv_label_create(container2, NULL);
  lv_label_set_style(t2, LV_PAGE_STYLE_BG, &titleStyle);
  lv_label_set_long_mode(t2, LV_LABEL_LONG_BREAK);
  lv_label_set_body_draw(t2, true);
  lv_obj_set_width(t2, LV_HOR_RES - (titleStyle.body.padding.left + titleStyle.body.padding.right) );
  lv_label_set_text(t2, "Alarm");


  lv_obj_t* l2 = lv_label_create(container2, NULL);
  lv_label_set_style(l2, LV_PAGE_STYLE_BG, &textStyle);
  lv_label_set_long_mode(l2, LV_LABEL_LONG_BREAK);
  lv_label_set_body_draw(l2, true);
  lv_obj_set_width(l2, LV_HOR_RES - (textStyle.body.padding.left + textStyle.body.padding.right) );
  lv_label_set_text(l2, "Missed: 09:30");

  /*****/
  lv_obj_t* container3 = lv_cont_create(page, NULL);
  lv_cont_set_style(container3, LV_CONT_STYLE_MAIN, &contStyle);
  lv_obj_set_width(container3, LV_HOR_RES);
  lv_obj_set_pos(container3, 0, lv_obj_get_y(container2) + lv_obj_get_height(container2)+5);
  lv_cont_set_layout(container3, LV_LAYOUT_COL_M);
  lv_cont_set_fit2(container3, LV_FIT_FLOOD, LV_FIT_TIGHT);
  lv_tileview_add_element(page, container3);

  lv_obj_t* t3 = lv_label_create(container3, NULL);
  lv_label_set_style(t3, LV_PAGE_STYLE_BG, &titleStyle);
  lv_label_set_long_mode(t3, LV_LABEL_LONG_BREAK);
  lv_label_set_body_draw(t3, true);
  lv_obj_set_width(t3, LV_HOR_RES - (titleStyle.body.padding.left + titleStyle.body.padding.right) );
  lv_label_set_text(t3, "Spotify");


  lv_obj_t* l3 = lv_label_create(container3, NULL);
  lv_label_set_style(l3, LV_PAGE_STYLE_BG, &textStyle);
  lv_label_set_long_mode(l3, LV_LABEL_LONG_BREAK);
  lv_label_set_body_draw(l3, true);
  lv_obj_set_width(l3, LV_HOR_RES - (textStyle.body.padding.left + textStyle.body.padding.right) );
  lv_label_set_text(l3, "Now playing: Bame game - Kanye West");









}
