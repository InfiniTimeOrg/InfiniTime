#include <libs/lvgl/lvgl.h>
#include "HeartRate.h"
#include <components/heartrate/HeartRateController.h>

#include "../DisplayApp.h"

using namespace Pinetime::Applications::Screens;

namespace {
  const char *ToString(Pinetime::Controllers::HeartRateController::States s) {
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

  static void btnStartStopEventHandler(lv_obj_t *obj, lv_event_t event) {
    HeartRate *screen = static_cast<HeartRate *>(obj->user_data);
    screen->OnStartStopEvent(event);
  }
}

HeartRate::HeartRate(Pinetime::Applications::DisplayApp *app, Controllers::HeartRateController& heartRateController) : Screen(app), heartRateController{heartRateController} {
  
  label_hr = lv_label_create(lv_scr_act(), NULL);

  lv_obj_set_style_local_text_font(label_hr, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_extrabold_compressed);

  lv_obj_align(label_hr, lv_scr_act(), LV_ALIGN_CENTER, -70, -40);
  lv_label_set_text(label_hr, "000");

  label_bpm = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_text(label_bpm, "Heart rate BPM");
  lv_obj_align(label_bpm, label_hr, LV_ALIGN_OUT_TOP_MID, 0, -20);


  label_status = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_text(label_status, ToString(Pinetime::Controllers::HeartRateController::States::NotEnoughData));
  
  lv_obj_align(label_status, label_hr, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

  btn_startStop = lv_btn_create(lv_scr_act(), NULL);
  btn_startStop->user_data = this;
  lv_obj_set_height(btn_startStop, 50);
  lv_obj_set_event_cb(btn_startStop, btnStartStopEventHandler);
  lv_obj_align(btn_startStop, nullptr, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  label_startStop = lv_label_create(btn_startStop, nullptr);
  UpdateStartStopButton(heartRateController.State() != Controllers::HeartRateController::States::Stopped);
}

HeartRate::~HeartRate() {
  lv_obj_clean(lv_scr_act());
}

bool HeartRate::Refresh() {
  char hr[4];

  auto state = heartRateController.State();
  switch(state) {
    case Controllers::HeartRateController::States::NoTouch:
    case Controllers::HeartRateController::States::NotEnoughData:
    case Controllers::HeartRateController::States::Stopped:
      lv_label_set_text(label_hr, "000");
      break;
    default:
      sprintf(hr, "%03d", heartRateController.HeartRate());
      lv_label_set_text(label_hr, hr);
  }

  lv_label_set_text(label_status, ToString(state));
  lv_obj_align(label_status, label_hr, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

  return running;
}

bool HeartRate::OnButtonPushed() {
  running = false;
  return true;
}

void HeartRate::OnStartStopEvent(lv_event_t event) {
  if (event == LV_EVENT_CLICKED) {
    if(heartRateController.State() == Controllers::HeartRateController::States::Stopped) {
      heartRateController.Start();
      UpdateStartStopButton(heartRateController.State() != Controllers::HeartRateController::States::Stopped);
    }
    else {
      heartRateController.Stop();
      UpdateStartStopButton(heartRateController.State() != Controllers::HeartRateController::States::Stopped);
    }
  }
}

void HeartRate::UpdateStartStopButton(bool isRunning) {
  if(isRunning)
    lv_label_set_text(label_startStop, "Stop");
  else
    lv_label_set_text(label_startStop, "Start");
}
