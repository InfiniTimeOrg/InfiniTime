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
  static constexpr int restoreLabelTimeoutSec = 2;
  static constexpr TickType_t restoreLabelTimeoutTicks = restoreLabelTimeoutSec * configTICK_RATE_HZ;

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
  lastLevel = Pinetime::Controllers::ImmediateAlertClient::Levels::NoAlert;

  container = lv_cont_create(lv_scr_act(), nullptr);

  lv_obj_set_size(container, LV_HOR_RES, LV_VER_RES);
  lv_obj_set_style_local_bg_color(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_set_style_local_pad_all(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_pad_inner(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_border_width(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

  lblTitle = lv_label_create(lv_scr_act(), nullptr);

  lv_label_set_text_static(lblTitle, defaultLabelText);
  lv_obj_set_style_local_text_color(lblTitle, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  lv_obj_align(lblTitle, nullptr, LV_ALIGN_CENTER, 0, -40);

  btnNone = lv_btn_create(container, nullptr);
  btnNone->user_data = this;
  lv_obj_set_event_cb(btnNone, btnImmediateAlertEventHandler);
  lv_obj_set_size(btnNone, 114, 76);
  lv_obj_align(btnNone, nullptr, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  lblNone = lv_label_create(btnNone, nullptr);
  lv_label_set_text_static(lblNone, noneLabelText);
  lv_obj_set_style_local_bg_color(btnNone, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);

  btnHigh = lv_btn_create(container, nullptr);
  btnHigh->user_data = this;
  lv_obj_set_event_cb(btnHigh, btnImmediateAlertEventHandler);
  lv_obj_set_size(btnHigh, 114, 76);
  lv_obj_align(btnHigh, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  lblHigh = lv_label_create(btnHigh, nullptr);
  lv_label_set_text_static(lblHigh, highLabelText);
  lv_obj_set_style_local_bg_color(btnHigh, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
}

FindMyPhone::~FindMyPhone() {
  lv_obj_clean(lv_scr_act());
}

void FindMyPhone::OnImmediateAlertEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (obj == btnNone) {
      lastLevel = Pinetime::Controllers::ImmediateAlertClient::Levels::NoAlert;
    } else if (obj == btnHigh) {
      lastLevel = Pinetime::Controllers::ImmediateAlertClient::Levels::HighAlert;
    }
    UpdateImmediateAlerts();
  }
}

void FindMyPhone::UpdateImmediateAlerts() {
  switch (lastLevel) {
    case Pinetime::Controllers::ImmediateAlertClient::Levels::NoAlert:
      lv_obj_set_style_local_text_color(lblTitle, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
      break;
    case Pinetime::Controllers::ImmediateAlertClient::Levels::HighAlert:
      lv_obj_set_style_local_text_color(lblTitle, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
      break;
    case Pinetime::Controllers::ImmediateAlertClient::Levels::MildAlert:
      // Not supported.
      ASSERT(false);
      break;
  }
  if (immediateAlertClient.SendImmediateAlert(lastLevel)) {
    lv_label_set_text_static(lblTitle, alertSentLabelText);
  } else {
    lv_label_set_text_static(lblTitle, noConnectionLabelText);
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
  lv_label_set_text_static(lblTitle, defaultLabelText);
  lv_obj_set_style_local_text_color(lblTitle, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
}
