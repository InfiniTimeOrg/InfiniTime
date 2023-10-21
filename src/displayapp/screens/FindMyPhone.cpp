#include "displayapp/screens/FindMyPhone.h"
#include <lvgl/lvgl.h>

#include "displayapp/DisplayApp.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void btnImmediateAlertEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<FindMyPhone*>(obj->user_data);
    screen->OnImmediateAlertEvent(obj, event);
  }
}

FindMyPhone::FindMyPhone(Pinetime::Controllers::ImmediateAlertClient& immediateAlertClient) : immediateAlertClient {immediateAlertClient} {
  last_level = Pinetime::Controllers::ImmediateAlertClient::Levels::NoAlert;

  container = lv_cont_create(lv_scr_act(), nullptr);

  lv_obj_set_size(container, LV_HOR_RES, LV_VER_RES);
  lv_obj_set_style_local_bg_color(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_set_style_local_pad_all(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_pad_inner(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_border_width(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

  label_title = lv_label_create(lv_scr_act(), nullptr);

  lv_label_set_text_static(label_title, "Find my phone");
  lv_obj_align(label_title, nullptr, LV_ALIGN_CENTER, 0, -40);

  bt_none = lv_btn_create(container, nullptr);
  bt_none->user_data = this;
  lv_obj_set_event_cb(bt_none, btnImmediateAlertEventHandler);
  lv_obj_set_size(bt_none, 76, 76);
  lv_obj_align(bt_none, nullptr, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  label_none = lv_label_create(bt_none, nullptr);
  lv_label_set_text_static(label_none, "None");
  lv_obj_set_style_local_bg_color(bt_none, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);

  bt_mild = lv_btn_create(container, nullptr);
  bt_mild->user_data = this;
  lv_obj_set_event_cb(bt_mild, btnImmediateAlertEventHandler);
  lv_obj_set_size(bt_mild, 76, 76);
  lv_obj_align(bt_mild, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  label_mild = lv_label_create(bt_mild, nullptr);
  lv_label_set_text_static(label_mild, "Mild");
  lv_obj_set_style_local_bg_color(bt_mild, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::highlight);

  bt_high = lv_btn_create(container, nullptr);
  bt_high->user_data = this;
  lv_obj_set_event_cb(bt_high, btnImmediateAlertEventHandler);
  lv_obj_set_size(bt_high, 76, 76);
  lv_obj_align(bt_high, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  label_high = lv_label_create(bt_high, nullptr);
  lv_label_set_text_static(label_high, "High");
  lv_obj_set_style_local_bg_color(bt_high, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);

  UpdateImmediateAlerts();
}

FindMyPhone::~FindMyPhone() {
  lv_obj_clean(lv_scr_act());
}

void FindMyPhone::OnImmediateAlertEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (obj == bt_none) {
      last_level = Pinetime::Controllers::ImmediateAlertClient::Levels::NoAlert;
    } else if (obj == bt_mild) {
      last_level = Pinetime::Controllers::ImmediateAlertClient::Levels::MildAlert;
    } else if (obj == bt_high) {
      last_level = Pinetime::Controllers::ImmediateAlertClient::Levels::HighAlert;
    }
    UpdateImmediateAlerts();
  }
}

void FindMyPhone::UpdateImmediateAlerts() {
  switch (last_level) {
    case Pinetime::Controllers::ImmediateAlertClient::Levels::NoAlert:
      lv_obj_set_style_local_text_color(label_title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
      break;
    case Pinetime::Controllers::ImmediateAlertClient::Levels::MildAlert:
      lv_obj_set_style_local_text_color(label_title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::highlight);
      break;
    case Pinetime::Controllers::ImmediateAlertClient::Levels::HighAlert:
      lv_obj_set_style_local_text_color(label_title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
      break;
  }
  immediateAlertClient.sendImmediateAlert(last_level);

}

