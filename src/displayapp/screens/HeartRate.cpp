#include <libs/lvgl/lvgl.h>
#include "HeartRate.h"
#include <components/heartrate/HeartRateController.h>

#include "../DisplayApp.h"

using namespace Pinetime::Applications::Screens;

namespace {
  const char* ToString(Pinetime::Controllers::HeartRateController::States s) {
    switch (s) {
      case Pinetime::Controllers::HeartRateController::States::NotEnoughData:
        return "Not enough data,\nplease wait...";
      case Pinetime::Controllers::HeartRateController::States::NoTouch:
        return "No touch detected";
      case Pinetime::Controllers::HeartRateController::States::Running:
        return "Measuring...";
      case Pinetime::Controllers::HeartRateController::States::Stopped:
        return "Stopped";
    }
    return "";
  }

  static void btnStartStopEventHandler(lv_event_t* event) {
    HeartRate* screen = static_cast<HeartRate*>(lv_event_get_user_data(event));
    screen->OnStartStopEvent(event);
  }
}

HeartRate::HeartRate(Pinetime::Applications::DisplayApp* app,
                     Controllers::HeartRateController& heartRateController,
                     System::SystemTask& systemTask)
  : Screen(app), heartRateController {heartRateController}, systemTask {systemTask} {
  bool isHrRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  label_hr = lv_label_create(lv_scr_act());

  lv_obj_set_style_text_font(label_hr, &jetbrains_mono_76, LV_PART_MAIN | LV_STATE_DEFAULT);

  if (isHrRunning)
    lv_obj_set_style_text_color(label_hr, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
  else
    lv_obj_set_style_text_color(label_hr, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN | LV_STATE_DEFAULT);

  lv_label_set_text(label_hr, "000");
  lv_obj_align(label_hr,  LV_ALIGN_CENTER, 0, -40);

  label_bpm = lv_label_create(lv_scr_act());
  lv_label_set_text(label_bpm, "Heart rate BPM");
  lv_obj_align_to(label_bpm, label_hr, LV_ALIGN_OUT_TOP_MID, 0, -20);

  label_status = lv_label_create(lv_scr_act());
  lv_obj_set_style_text_color(label_status, lv_color_hex(0x222222), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_label_set_text(label_status, ToString(Pinetime::Controllers::HeartRateController::States::NotEnoughData));

  lv_obj_align_to(label_status, label_hr, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

  btn_startStop = lv_btn_create(lv_scr_act());
  btn_startStop->user_data = this;
  lv_obj_set_height(btn_startStop, 50);
  lv_obj_add_event_cb(btn_startStop, btnStartStopEventHandler, LV_EVENT_ALL, btn_startStop->user_data);
  lv_obj_align(btn_startStop, LV_ALIGN_BOTTOM_MID, 0, 0);

  label_startStop = lv_label_create(btn_startStop);
  UpdateStartStopButton(isHrRunning);
  if (isHrRunning)
    systemTask.PushMessage(Pinetime::System::Messages::DisableSleeping);

  taskRefresh = lv_timer_create(RefreshTaskCallback, 100, this);
}

HeartRate::~HeartRate() {
  lv_timer_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
  systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
}

void HeartRate::Refresh() {

  auto state = heartRateController.State();
  
  switch (state) {
    case Controllers::HeartRateController::States::NoTouch:
    case Controllers::HeartRateController::States::NotEnoughData:
      // case Controllers::HeartRateController::States::Stopped:
      if (state != prevState) {
        lv_label_set_text(label_hr, "000");
      }
      break;
    default:
      uint8_t heartRate = heartRateController.HeartRate();
      if (heartRate != prevHeartRate) {
        lv_label_set_text_fmt(label_hr, "%03d", heartRate);
        prevHeartRate = heartRate;
      }
  }

  if (state != prevState) {
    lv_label_set_text(label_status, ToString(state));
    lv_obj_align_to(label_status, label_hr, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
  }
  prevState = state;
}

void HeartRate::OnStartStopEvent(lv_event_t* event) {
  if (lv_event_get_code(event) == LV_EVENT_CLICKED) {
    if (heartRateController.State() == Controllers::HeartRateController::States::Stopped) {
      heartRateController.Start();
      UpdateStartStopButton(heartRateController.State() != Controllers::HeartRateController::States::Stopped);
      systemTask.PushMessage(Pinetime::System::Messages::DisableSleeping);
      lv_obj_set_style_text_color(label_hr, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN | LV_STATE_DEFAULT);
    } else {
      heartRateController.Stop();
      UpdateStartStopButton(heartRateController.State() != Controllers::HeartRateController::States::Stopped);
      systemTask.PushMessage(Pinetime::System::Messages::EnableSleeping);
      lv_obj_set_style_text_color(label_hr, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN | LV_STATE_DEFAULT);
    }
  }
}

void HeartRate::UpdateStartStopButton(bool isRunning) {
  if (isRunning)
    lv_label_set_text(label_startStop, "Stop");
  else
    lv_label_set_text(label_startStop, "Start");
}
