#include "displayapp/screens/FindMyPhone.h"
#include <lvgl/lvgl.h>

#include "displayapp/DisplayApp.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  static constexpr char alertSentLabelText[] = "Alerting";

  static constexpr char stopLabelText[] = "Stop";
  static constexpr char ringLabelText[] = "Ring";

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

  btnStop = lv_btn_create(container, nullptr);
  btnStop->user_data = this;
  lv_obj_set_event_cb(btnStop, btnImmediateAlertEventHandler);
  lv_obj_set_size(btnStop, 114, 76);
  lv_obj_align(btnStop, nullptr, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  lblStop = lv_label_create(btnStop, nullptr);
  lv_label_set_text_static(lblStop, stopLabelText);
  lv_obj_set_style_local_bg_color(btnStop, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::bgAlt);

  btnRing = lv_btn_create(container, nullptr);
  btnRing->user_data = this;
  lv_obj_set_event_cb(btnRing, btnImmediateAlertEventHandler);
  lv_obj_set_size(btnRing, 114, 76);
  lv_obj_align(btnRing, nullptr, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  lblRing = lv_label_create(btnRing, nullptr);
  lv_label_set_text_static(lblRing, ringLabelText);
  lv_obj_set_style_local_bg_color(btnRing, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_RED);
  refreshTask = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
}

FindMyPhone::~FindMyPhone() {
  lv_task_del(refreshTask);
  lv_obj_clean(lv_scr_act());
}

void FindMyPhone::OnImmediateAlertEvent(lv_obj_t* obj, lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (obj == btnStop) {
      lastUserInitiatedLevel = Pinetime::Controllers::ImmediateAlertClient::Levels::NoAlert;
    } else if (obj == btnRing) {
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
    lv_obj_clear_state(btnStop, LV_STATE_DISABLED);
    lv_obj_clear_state(btnRing, LV_STATE_DISABLED);
  } else {
    lv_obj_add_state(btnStop, LV_STATE_DISABLED);
    lv_obj_add_state(btnRing, LV_STATE_DISABLED);
    lastUserInitiatedLevel = std::nullopt;
  }
  const auto& label_state = GetLabelState();
  lv_obj_set_style_local_text_color(lblTitle, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, label_state.color);
  lv_label_set_text_static(lblTitle, label_state.text);
  lv_obj_align(lblTitle, nullptr, LV_ALIGN_CENTER, 0, -40);
}
