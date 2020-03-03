#include <cstdio>
#include <libs/date/includes/date/date.h>
#include <Components/DateTime/DateTimeController.h>
#include <Version.h>
#include <libs/lvgl/src/lv_core/lv_obj.h>
#include <libs/lvgl/src/lv_font/lv_font.h>
#include <libs/lvgl/lvgl.h>
#include <libraries/log/nrf_log.h>
#include "Message.h"
#include <DisplayApp/DisplayApp.h>


using namespace Pinetime::Applications::Screens;

extern lv_font_t jetbrains_mono_bold_20;

static void event_handler(lv_obj_t * obj, lv_event_t event) {
  Message* screen = static_cast<Message *>(obj->user_data);
  screen->OnObjectEvent(obj, event);
}

Message::Message(DisplayApp* app) : Screen(app) {

  backgroundLabel = lv_label_create(lv_scr_act(), NULL);
  backgroundLabel->user_data = this;

  labelStyle = const_cast<lv_style_t *>(lv_label_get_style(backgroundLabel, LV_LABEL_STYLE_MAIN));
  labelStyle->text.font = &jetbrains_mono_bold_20;

  lv_label_set_style(backgroundLabel, LV_LABEL_STYLE_MAIN, labelStyle);
  lv_obj_set_click(backgroundLabel, true);
  lv_obj_set_event_cb(backgroundLabel, event_handler);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text(backgroundLabel, "");
//  lv_obj_align(backgroundLabel, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  button = lv_btn_create(lv_scr_act(), NULL);
  lv_obj_set_event_cb(button, event_handler);
  lv_obj_align(button, NULL, LV_ALIGN_CENTER, 0, -40);
  button->user_data = this;

  label = lv_label_create(button, NULL);
  lv_label_set_style(label, LV_LABEL_STYLE_MAIN, labelStyle);
  lv_label_set_text(label, "Hello!");

  labelClick = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_style(labelClick, LV_LABEL_STYLE_MAIN, labelStyle);
  lv_obj_align(labelClick, button, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);
  lv_label_set_text(labelClick, "0");
}

Message::~Message() {
  lv_obj_clean(lv_scr_act());
}

bool Message::Refresh() {
  if(previousClickCount != clickCount) {
    lv_label_set_text_fmt(labelClick, "%d", clickCount);
    previousClickCount = clickCount;
  }

  return running;
}

void Message::OnObjectEvent(lv_obj_t *obj, lv_event_t event) {
  if(obj == backgroundLabel) {
    if(event == LV_EVENT_CLICKED) {
      app->PushMessage(DisplayApp::Messages::SwitchScreen);
      NRF_LOG_INFO("SCREEN");
    }
    return ;
  }

  if(event == LV_EVENT_CLICKED) {
    NRF_LOG_INFO("Clicked");
    clickCount++;
  }
  else if(event == LV_EVENT_VALUE_CHANGED) {
    NRF_LOG_INFO("Toggled");
  }
}

bool Message::OnButtonPushed() {
  running = false;
  return true;
}
