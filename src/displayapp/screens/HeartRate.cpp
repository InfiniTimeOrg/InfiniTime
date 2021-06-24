#include <libs/lvgl/lvgl.h>
#include "HeartRate.h"
#include <components/heartrate/HeartRateController.h>
#include "lv_i18n/lv_i18n.h"

#include "../DisplayApp.h"

using namespace Pinetime::Applications::Screens;

namespace {
  const char* ToString(Pinetime::Controllers::HeartRateController::States s) {
    switch (s) {
      case Pinetime::Controllers::HeartRateController::States::NotEnoughData:
        return _("heartrate_notenoughdata") ;
      case Pinetime::Controllers::HeartRateController::States::NoTouch:
        return _("heartrate_notouchdetected");
      case Pinetime::Controllers::HeartRateController::States::Running:
        return _("heartrate_measuring");
      case Pinetime::Controllers::HeartRateController::States::Stopped:
        return _("heartrate_stopped");
    }
    return "";
  }

  static void btnStartStopEventHandler(lv_obj_t* obj, lv_event_t event) {
    HeartRate* screen = static_cast<HeartRate*>(obj->user_data);
    screen->OnStartStopEvent(event);
  }
}

HeartRate::HeartRate(Pinetime::Applications::DisplayApp* app,
                     Controllers::HeartRateController& heartRateController,
                     System::SystemTask& systemTask)
  : Screen(app), heartRateController {heartRateController}, systemTask {systemTask} {
  bool isHrRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  label_hr = lv_label_create(lv_scr_act(), nullptr);

  lv_obj_set_style_local_text_font(label_hr, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);

  if (isHrRunning)
    lv_obj_set_style_local_text_color(label_hr, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
  else
    lv_obj_set_style_local_text_color(label_hr, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);

  lv_label_set_text(label_hr, "000");
  lv_obj_align(label_hr, nullptr, LV_ALIGN_CENTER, 0, -40);

  label_bpm = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(label_bpm, _("heartrate_bpm_title"));
  lv_obj_align(label_bpm, label_hr, LV_ALIGN_OUT_TOP_MID, 0, -20);

  label_status = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(label_status, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x222222));
  lv_label_set_text(label_status, ToString(Pinetime::Controllers::HeartRateController::States::NotEnoughData));

  lv_obj_align(label_status, label_hr, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

  btn_startStop = lv_btn_create(lv_scr_act(), nullptr);
  btn_startStop->user_data = this;
  lv_obj_set_height(btn_startStop, 50);
  lv_obj_set_event_cb(btn_startStop, btnStartStopEventHandler);
  lv_obj_align(btn_startStop, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  label_startStop = lv_label_create(btn_startStop, nullptr);
  UpdateStartStopButton(isHrRunning);
  if (isHrRunning)
    systemTask.PushMessage(Pinetime::System::Messages::DisableSleeping);
}

HeartRate::~HeartRate() {
  lv_obj_clean(lv_scr_act());
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
}

bool HeartRate::Refresh() {

  auto state = heartRateController.State();
  switch (state) {
    case Controllers::HeartRateController::States::NoTouch:
    case Controllers::HeartRateController::States::NotEnoughData:
      // case Controllers::HeartRateController::States::Stopped:
      lv_label_set_text(label_hr, "000");
      break;
    default:
      lv_label_set_text_fmt(label_hr, "%03d", heartRateController.HeartRate());
  }

  lv_label_set_text(label_status, ToString(state));
  lv_obj_align(label_status, label_hr, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

  return running;
}

void HeartRate::OnStartStopEvent(lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if (heartRateController.State() == Controllers::HeartRateController::States::Stopped) {
      heartRateController.Start();
      UpdateStartStopButton(heartRateController.State() != Controllers::HeartRateController::States::Stopped);
      systemTask.PushMessage(Pinetime::System::Messages::DisableSleeping);
      lv_obj_set_style_local_text_color(label_hr, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
    } else {
      heartRateController.Stop();
      UpdateStartStopButton(heartRateController.State() != Controllers::HeartRateController::States::Stopped);
      systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
      lv_obj_set_style_local_text_color(label_hr, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GRAY);
    }
  }
}

void HeartRate::UpdateStartStopButton(bool isRunning) {
  if (isRunning)
    lv_label_set_text(label_startStop, _("heartrate_stop"));
  else
    lv_label_set_text(label_startStop, _("heartrate_start"));
}
