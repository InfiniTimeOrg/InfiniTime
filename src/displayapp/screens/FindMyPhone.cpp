#include "displayapp/screens/FindMyPhone.h"
#include <lvgl/lvgl.h>

#include "displayapp/DisplayApp.h"
#include "displayapp/InfiniTimeTheme.h"

using namespace Pinetime::Applications::Screens;

namespace {
  void btnStartStopEventHandler(lv_obj_t* obj, lv_event_t event) {
    auto* screen = static_cast<FindMyPhone*>(obj->user_data);
    screen->OnStartStopEvent(event);
  }
}

FindMyPhone::FindMyPhone(Pinetime::Controllers::ImmediateAlertService& immediateAlertService)
  : immediateAlertService {immediateAlertService} {
  isFindMyPhoneRunning = false;
  label_title = lv_label_create(lv_scr_act(), nullptr);

  lv_label_set_text_static(label_title, "Find my phone");
  lv_obj_align(label_title, nullptr, LV_ALIGN_CENTER, 0, -40);

  btn_startStop = lv_btn_create(lv_scr_act(), nullptr);
  btn_startStop->user_data = this;
  lv_obj_set_height(btn_startStop, 50);
  lv_obj_set_event_cb(btn_startStop, btnStartStopEventHandler);
  lv_obj_align(btn_startStop, label_title, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

  label_startStop = lv_label_create(btn_startStop, nullptr);
  UpdateStartStopButton(isFindMyPhoneRunning);
}

FindMyPhone::~FindMyPhone() {
  lv_obj_clean(lv_scr_act());
}

void FindMyPhone::OnStartStopEvent(lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    isFindMyPhoneRunning = !isFindMyPhoneRunning;
    UpdateStartStopButton(isFindMyPhoneRunning);
  }
}

void FindMyPhone::UpdateStartStopButton(bool isRunning) {
  if (isRunning) {
    immediateAlertService.sendImmediateAlert(Pinetime::Controllers::ImmediateAlertService::Levels::HighAlert);
    lv_obj_set_style_local_text_color(label_title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::highlight);
    lv_label_set_text_static(label_startStop, "Stop");
  } else {
    lv_obj_set_style_local_text_color(label_title, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, Colors::lightGray);
    lv_label_set_text_static(label_startStop, "Start");
    immediateAlertService.sendImmediateAlert(Pinetime::Controllers::ImmediateAlertService::Levels::NoAlert);
  }
}
