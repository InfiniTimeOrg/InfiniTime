#include "displayapp/screens/FindMyPhone.h"
#include <lvgl/lvgl.h>

#include "displayapp/DisplayApp.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static constexpr char alertSentLabelText[] = "Alerting";

  static constexpr char noneLabelText[] = "Stop";
  static constexpr char highLabelText[] = "Ring";

  void btnImmediateAlertEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<FindMyPhone*>(obj->user_data);
    screen->OnImmediateAlertEvent(obj, event);
  }
}

const FindMyPhone::LabelState FindMyPhone::stoppedLabelState {
  .text = "Alert stopped",
  .color = LV_COLOR_WHITE,
};

const FindMyPhone::LabelState FindMyPhone::noConnectionLabelState {
  .text = "No connection",
  .color = LV_COLOR_WHITE,
};

const FindMyPhone::LabelState FindMyPhone::noServiceLabelState {
  .text = "No service",
  .color = LV_COLOR_WHITE,
};

const FindMyPhone::LabelState FindMyPhone::defaultLabelState {
  .text = "Ready",
  .color = LV_COLOR_WHITE,
};

const FindMyPhone::LabelState FindMyPhone::alertingLabelState {
  .text = "Alerting",
  .color = LV_COLOR_RED,
};

FindMyPhone::FindMyPhone(Pinetime::Controllers::ImmediateAlertClient& immediateAlertClient) : immediateAlertClient {immediateAlertClient} {
  container = lv_cont_create(lv_scr_act(), nullptr);

  lv_obj_set_size(container, LV_HOR_RES, LV_VER_RES);
  lv_obj_set_style_local_bg_color(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  lv_obj_set_style_local_pad_all(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_pad_inner(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);
  lv_obj_set_style_local_border_width(container, LV_CONT_PART_MAIN, LV_STATE_DEFAULT, 0);

  lblTitle = lv_label_create(lv_scr_act(), nullptr);

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
  refreshTask = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

FindMyPhone::~FindMyPhone() {
  lv_task_del(refreshTask);
  lv_obj_clean(lv_scr_act());
}

void FindMyPhone::OnImmediateAlertEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (obj == btnNone) {
      lastUserInitiatedLevel = Pinetime::Controllers::ImmediateAlertClient::Levels::NoAlert;
    } else if (obj == btnHigh) {
      lastUserInitiatedLevel = Pinetime::Controllers::ImmediateAlertClient::Levels::HighAlert;
    } else {
      // Unknown button?
      ASSERT(false);
      return;
    }
    immediateAlertClient.SendImmediateAlert(*lastUserInitiatedLevel);
  }
}

const FindMyPhone::LabelState& FindMyPhone::GetLabelState() const {
  const auto service_state = immediateAlertClient.GetState();
  switch (service_state) {
    case Pinetime::Controllers::ImmediateAlertClient::State::NoConnection:
      return noConnectionLabelState;
    case Pinetime::Controllers::ImmediateAlertClient::State::NoIAS:
      return noServiceLabelState;
    case Pinetime::Controllers::ImmediateAlertClient::State::Connected:
      break;
  }
  // Conntected state handling.
  if (!lastUserInitiatedLevel.has_value()) {
    return defaultLabelState;
  }
  switch (*lastUserInitiatedLevel) {
    case Pinetime::Controllers::ImmediateAlertClient::Levels::NoAlert:
      return stoppedLabelState;
    case Pinetime::Controllers::ImmediateAlertClient::Levels::HighAlert:
      return alertingLabelState;
    case Pinetime::Controllers::ImmediateAlertClient::Levels::MildAlert:
      // Not supported
    default:
      ASSERT(false);
      return alertingLabelState;
  }
}

void FindMyPhone::Refresh() {
  const auto service_state = immediateAlertClient.GetState();
  if (service_state == Pinetime::Controllers::ImmediateAlertClient::State::Connected) {
    lv_obj_clear_state(btnNone, LV_STATE_DISABLED);
    lv_obj_clear_state(btnHigh, LV_STATE_DISABLED);
  } else {
    lv_obj_add_state(btnNone, LV_STATE_DISABLED);
    lv_obj_add_state(btnHigh, LV_STATE_DISABLED);
    lastUserInitiatedLevel = std::nullopt;
  }
  const auto& label_state = GetLabelState();
  lv_obj_set_style_local_text_color(lblTitle, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, label_state.color);
  lv_label_set_text_static(lblTitle, label_state.text);
  lv_obj_align(lblTitle, nullptr, LV_ALIGN_CENTER, 0, -40);
}
