#include <libs/lvgl/lvgl.h>
#include <displayapp/DisplayApp.h>
#include <functional>
#include "Notifications.h"
using namespace Pinetime::Applications::Screens;

Notifications::Notifications(DisplayApp* app) : Screen(app), screens{app, {
        [this]() -> std::unique_ptr<Screen> { return CreateScreen1(); },
        [this]() -> std::unique_ptr<Screen> { return CreateScreen2(); },
        [this]() -> std::unique_ptr<Screen> { return CreateScreen3(); }
}
} {

}

Notifications::~Notifications() {
  lv_obj_clean(lv_scr_act());
}

bool Notifications::Refresh() {
  screens.Refresh();
  return running;
}

bool Notifications::OnTouchEvent(Pinetime::Applications::TouchEvents event) {
  return screens.OnTouchEvent(event);
}


bool Notifications::OnButtonPushed() {
  running = false;
  return true;
}

std::unique_ptr<Screen> Notifications::CreateScreen1() {
  return std::unique_ptr<Screen>(new Notifications::NotificationItem(app, "Message", "Marcel Pickett: Did you bring your ticket?", "Shot notif", "Short text", 1, 3));
}

std::unique_ptr<Screen> Notifications::CreateScreen2() {
  return std::unique_ptr<Screen>(new Notifications::NotificationItem(app, "Alarm", "Missed: 09:30", 2, 3));
}

std::unique_ptr<Screen> Notifications::CreateScreen3() {
  return std::unique_ptr<Screen>(new Notifications::NotificationItem(app, "Spotify", "Now playing: Bame game - Kanye West", 3, 3));
}

Notifications::NotificationItem::NotificationItem(Pinetime::Applications::DisplayApp *app, const char *title, const char* msg, uint8_t notifNr, uint8_t notifNb) :
        Screen(app), notifNr{notifNr}, notifNb{notifNb} {

  lv_obj_t* container1 = lv_cont_create(lv_scr_act(), NULL);
  static lv_style_t contStyle;
  lv_style_copy(&contStyle, lv_cont_get_style(container1, LV_CONT_STYLE_MAIN));
  contStyle.body.padding.inner = 20;
  lv_cont_set_style(container1, LV_CONT_STYLE_MAIN, &contStyle);
  lv_obj_set_width(container1, LV_HOR_RES);
  lv_obj_set_height(container1, LV_VER_RES);
  lv_obj_set_pos(container1, 0, 0);
  lv_cont_set_layout(container1, LV_LAYOUT_OFF);
  lv_cont_set_fit2(container1, LV_FIT_FLOOD, LV_FIT_FLOOD);



  lv_obj_t* t1 = lv_label_create(container1, NULL);

  static lv_style_t titleStyle;
  static lv_style_t textStyle;
  static lv_style_t bottomStyle;
  lv_style_copy(&titleStyle, lv_label_get_style(t1, LV_LABEL_STYLE_MAIN));
  lv_style_copy(&textStyle, lv_label_get_style(t1, LV_LABEL_STYLE_MAIN));
  lv_style_copy(&bottomStyle, lv_label_get_style(t1, LV_LABEL_STYLE_MAIN));
  titleStyle.body.padding.inner = 5;
  textStyle.body.padding.inner = 5;
  titleStyle.body.grad_color = LV_COLOR_GRAY;
  titleStyle.body.main_color = LV_COLOR_GRAY;
  textStyle.body.border.part = LV_BORDER_NONE;

  //bottomStyle.body.padding.inner = 5;
  bottomStyle.body.main_color = LV_COLOR_GREEN;
  bottomStyle.body.grad_color = LV_COLOR_GREEN;
  bottomStyle.body.border.part = LV_BORDER_TOP;
  bottomStyle.body.border.color = LV_COLOR_RED;


  lv_label_set_style(t1, LV_LABEL_STYLE_MAIN, &titleStyle);
  lv_label_set_long_mode(t1, LV_LABEL_LONG_BREAK);
  lv_label_set_body_draw(t1, true);
  lv_obj_set_width(t1, LV_HOR_RES - (titleStyle.body.padding.left + titleStyle.body.padding.right) );
  lv_label_set_text(t1, title);
  lv_obj_set_pos(t1, titleStyle.body.padding.left, titleStyle.body.padding.top);

  auto titleHeight = lv_obj_get_height(t1);

  lv_obj_t* l1 = lv_label_create(container1, NULL);
  lv_label_set_style(l1, LV_LABEL_STYLE_MAIN, &textStyle);
  lv_obj_set_pos(l1, textStyle.body.padding.left, titleHeight + titleStyle.body.padding.bottom + textStyle.body.padding.bottom + textStyle.body.padding.top);

  lv_label_set_long_mode(l1, LV_LABEL_LONG_BREAK);
  lv_label_set_body_draw(l1, true);
  lv_obj_set_width(l1, LV_HOR_RES - (textStyle.body.padding.left + textStyle.body.padding.right) );
  lv_label_set_text(l1, msg);

  lv_obj_t* bottomlabel = lv_label_create(container1, NULL);
  lv_label_set_style(bottomlabel, LV_LABEL_STYLE_MAIN, &bottomStyle);
  lv_obj_set_width(bottomlabel, LV_HOR_RES - (bottomStyle.body.padding.left + bottomStyle.body.padding.right) );
  snprintf(pageText, 4,  "%d/%d", notifNr, notifNb);
  lv_label_set_text(bottomlabel, pageText);
  auto bottomHeight = lv_obj_get_height(bottomlabel);
  lv_obj_set_pos(bottomlabel, 0, LV_VER_RES - (bottomHeight*2));
}

Notifications::NotificationItem::NotificationItem(Pinetime::Applications::DisplayApp *app, const char *title1, const char* msg1, const char *title2, const char* msg2, uint8_t notifNr, uint8_t notifNb) :
        Screen(app), notifNr{notifNr}, notifNb{notifNb} {

  lv_obj_t* container1 = lv_cont_create(lv_scr_act(), NULL);
  static lv_style_t contStyle;
  lv_style_copy(&contStyle, lv_cont_get_style(container1, LV_CONT_STYLE_MAIN));
  contStyle.body.padding.inner = 20;
  lv_cont_set_style(container1, LV_CONT_STYLE_MAIN, &contStyle);
  lv_obj_set_width(container1, LV_HOR_RES);
  lv_obj_set_height(container1, LV_VER_RES);
  lv_obj_set_pos(container1, 0, 0);
  lv_cont_set_layout(container1, LV_LAYOUT_OFF);
  lv_cont_set_fit2(container1, LV_FIT_FLOOD, LV_FIT_FLOOD);



  lv_obj_t* t1 = lv_label_create(container1, NULL);

  static lv_style_t titleStyle;
  static lv_style_t textStyle;
  static lv_style_t bottomStyle;
  lv_style_copy(&titleStyle, lv_label_get_style(t1, LV_LABEL_STYLE_MAIN));
  lv_style_copy(&textStyle, lv_label_get_style(t1, LV_LABEL_STYLE_MAIN));
  lv_style_copy(&bottomStyle, lv_label_get_style(t1, LV_LABEL_STYLE_MAIN));
  titleStyle.body.padding.inner = 5;
  textStyle.body.padding.inner = 5;
  titleStyle.body.grad_color = LV_COLOR_GRAY;
  titleStyle.body.main_color = LV_COLOR_GRAY;
  textStyle.body.border.part = LV_BORDER_NONE;

  //bottomStyle.body.padding.inner = 5;
  bottomStyle.body.main_color = LV_COLOR_GREEN;
  bottomStyle.body.grad_color = LV_COLOR_GREEN;
  bottomStyle.body.border.part = LV_BORDER_TOP;
  bottomStyle.body.border.color = LV_COLOR_RED;


  lv_label_set_style(t1, LV_LABEL_STYLE_MAIN, &titleStyle);
  lv_label_set_long_mode(t1, LV_LABEL_LONG_BREAK);
  lv_label_set_body_draw(t1, true);
  lv_obj_set_width(t1, LV_HOR_RES - (titleStyle.body.padding.left + titleStyle.body.padding.right) );
  lv_label_set_text(t1, title1);
  lv_obj_set_pos(t1, titleStyle.body.padding.left, titleStyle.body.padding.top);

  auto titleHeight = lv_obj_get_height(t1);

  lv_obj_t* l1 = lv_label_create(container1, NULL);
  lv_label_set_style(l1, LV_LABEL_STYLE_MAIN, &textStyle);
  lv_obj_set_pos(l1, textStyle.body.padding.left, titleHeight + titleStyle.body.padding.bottom + textStyle.body.padding.bottom + textStyle.body.padding.top);

  lv_label_set_long_mode(l1, LV_LABEL_LONG_BREAK);
  lv_label_set_body_draw(l1, true);
  lv_obj_set_width(l1, LV_HOR_RES - (textStyle.body.padding.left + textStyle.body.padding.right) );
  lv_label_set_text(l1, msg1);

  auto bottomPos = lv_obj_get_y(l1) + lv_obj_get_height(l1) + textStyle.body.padding.bottom;

  /*
  lv_obj_t* bottomlabel = lv_label_create(container1, NULL);
  lv_label_set_style(bottomlabel, LV_LABEL_STYLE_MAIN, &bottomStyle);
  lv_obj_set_width(bottomlabel, LV_HOR_RES - (bottomStyle.body.padding.left + bottomStyle.body.padding.right) );
  snprintf(pageText, 4,  "%d/%d", notifNr, notifNb);
  lv_label_set_text(bottomlabel, pageText);
  auto bottomHeight = lv_obj_get_height(bottomlabel);
  lv_obj_set_pos(bottomlabel, 0, LV_VER_RES - (bottomHeight*2));
   */

  //-------------------------------------------------
/*
  lv_obj_t* container2 = lv_cont_create(lv_scr_act(), NULL);
  lv_cont_set_style(container2, LV_CONT_STYLE_MAIN, &contStyle);
  lv_obj_set_width(container2, LV_HOR_RES);
  lv_obj_set_height(container2, LV_VER_RES - bottomPos);
  lv_obj_set_pos(container2, 0, bottomPos);
  lv_cont_set_layout(container2, LV_LAYOUT_OFF);
  lv_cont_set_fit2(container2, LV_FIT_FLOOD, LV_FIT_FLOOD);
*/
  lv_obj_t* t2 = lv_label_create(container1, NULL);


  lv_label_set_style(t2, LV_LABEL_STYLE_MAIN, &titleStyle);
  lv_label_set_long_mode(t2, LV_LABEL_LONG_BREAK);
  lv_label_set_body_draw(t2, true);
  lv_obj_set_width(t2, LV_HOR_RES - (titleStyle.body.padding.left + titleStyle.body.padding.right) );
  lv_label_set_text(t2, title2);
  lv_obj_set_pos(t2, titleStyle.body.padding.left, bottomPos + titleStyle.body.padding.top);

  auto title2Height = lv_obj_get_height(t2);

  lv_obj_t* l2 = lv_label_create(container1, NULL);
  lv_label_set_style(l2, LV_LABEL_STYLE_MAIN, &textStyle);
  lv_obj_set_pos(l2, textStyle.body.padding.left, bottomPos + title2Height + titleStyle.body.padding.bottom + textStyle.body.padding.bottom + textStyle.body.padding.top);

  lv_label_set_long_mode(l2, LV_LABEL_LONG_BREAK);
  lv_label_set_body_draw(l2, true);
  lv_obj_set_width(l2, LV_HOR_RES - (textStyle.body.padding.left + textStyle.body.padding.right) );
  lv_label_set_text(l2, msg2);

}

Notifications::NotificationItem::~NotificationItem() {
  lv_obj_clean(lv_scr_act());
}
