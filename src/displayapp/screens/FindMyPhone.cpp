#include "displayapp/screens/FindMyPhone.h"
#include <lvgl/lvgl.h>

#include "displayapp/DisplayApp.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static constexpr char defaultLabelText[] = "Find my phone";
  static constexpr char alertSentLabelText[] = "Alert sent";
  static constexpr char noConnectionLabelText[] = "No connection";
  static constexpr char noneLabelText[] = "Stop";
  static constexpr char highLabelText[] = "Ring";
  static constexpr auto restoreLabelTimeoutTicks = pdMS_TO_TICKS(2 * 1000);

  void btnImmediateAlertEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<FindMyPhone*>(obj->user_data);
    screen->OnImmediateAlertEvent(obj, event);
  }

  void RestoreLabelTaskCallback(lv_task_t* task) {
    auto* screen = static_cast<FindMyPhone*>(task->user_data);
    screen->RestoreLabelText();
    screen->StopRestoreLabelTask();
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

  lv_label_set_text_static(label_title, defaultLabelText);
  lv_obj_set_style_local_text_color(label_title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
  lv_obj_align(label_title, nullptr, LV_ALIGN_CENTER, 0, -40);

  bt_none = lv_btn_create(container, nullptr);
  bt_none->user_data = this;
  lv_obj_set_event_cb(bt_none, btnImmediateAlertEventHandler);
  lv_obj_set_size(bt_none, 114, 76);
  lv_obj_align(bt_none, nullptr, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  label_none = lv_label_create(bt_none, nullptr);
  lv_label_set_text_static(label_none, noneLabelText);
  lv_obj_set_style_local_bg_color(bt_none, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);

  bt_high = lv_btn_create(container, nullptr);
  bt_high->user_data = this;
  lv_obj_set_event_cb(bt_high, btnImmediateAlertEventHandler);
  lv_obj_set_size(bt_high, 114, 76);
  lv_obj_align(bt_high, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  label_high = lv_label_create(bt_high, nullptr);
  lv_label_set_text_static(label_high, highLabelText);
  lv_obj_set_style_local_bg_color(bt_high, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
}

FindMyPhone::~FindMyPhone() {
  lv_obj_clean(lv_scr_act());
}

void FindMyPhone::OnImmediateAlertEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (obj == bt_none) {
      last_level = Pinetime::Controllers::ImmediateAlertClient::Levels::NoAlert;
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
    case Pinetime::Controllers::ImmediateAlertClient::Levels::HighAlert:
      lv_obj_set_style_local_text_color(label_title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
      break;
    case Pinetime::Controllers::ImmediateAlertClient::Levels::MildAlert:
      // Not supported.
      ASSERT(false);
      break;
  }
  if (immediateAlertClient.sendImmediateAlert(last_level)) {
    lv_label_set_text_static(label_title, alertSentLabelText);
  } else {
    lv_label_set_text_static(label_title, noConnectionLabelText);
  }
  ScheduleRestoreLabelTask();
}

void FindMyPhone::ScheduleRestoreLabelTask() {
  if (taskRestoreLabelText) {
    return;
  }
  taskRestoreLabelText = lv_task_create(RestoreLabelTaskCallback, restoreLabelTimeoutTicks, LV_TASK_PRIO_MID, this);
}

void FindMyPhone::StopRestoreLabelTask() {
  if (taskRestoreLabelText) {
    lv_task_del(taskRestoreLabelText);
    taskRestoreLabelText = nullptr;
  }
}

void FindMyPhone::RestoreLabelText() {
  lv_label_set_text_static(label_title, defaultLabelText);
  lv_obj_set_style_local_text_color(label_title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
}
