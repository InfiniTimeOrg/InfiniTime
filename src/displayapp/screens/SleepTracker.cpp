#include "displayapp/screens/SleepTracker.h"

#include <lvgl/lvgl.h>
#include <components/heartrate/HeartRateController.h>
#include <components/datetime/DateTimeController.h>
#include <nrf_log.h>

using namespace Pinetime::Applications::Screens;

namespace {

  void BpmDataCallback(lv_task_t* task) {
    auto* screen = static_cast<SleepTracker*>(task->user_data);
    screen->GetBPM();
  }

}

SleepTracker::SleepTracker(Controllers::HeartRateController& heartRateController, Controllers::DateTime& dateTimeController, System::SystemTask& systemTask)
  : heartRateController {heartRateController}, dateTimeController {dateTimeController}, wakeLock(systemTask) {

  wakeLock.Lock();
  
  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "My test application");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

  label_hr = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(label_hr, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);

  // Create the refresh task
  mainRefreshTask = lv_task_create(RefreshTaskCallback, 100, LV_TASK_PRIO_MID, this);
  hrRefreshTask = lv_task_create(BpmDataCallback, 3000, LV_TASK_PRIO_MID, this);
}

SleepTracker::~SleepTracker() {
  wakeLock.Release();

  lv_obj_clean(lv_scr_act());
  lv_task_del(mainRefreshTask);
  lv_task_del(hrRefreshTask);
}

// This function is called periodically from the refresh task
void SleepTracker::Refresh() {
  // Get the current heart rate
}

void SleepTracker::GetBPM() {
  // Get the heart rate from the controller
  prevBpm = bpm;
  bpm = heartRateController.HeartRate();

  if(prevBpm != 0)
    rollingBpm = (rollingBpm + bpm) / 2;
  else
    rollingBpm = bpm;

  // Get the current time from DateTimeController
  //auto now = dateTimeController.CurrentDateTime();
  int hours = dateTimeController.Hours();
  int minutes = dateTimeController.Minutes();
  int seconds = dateTimeController.Seconds();

  // Log the BPM and current time
  NRF_LOG_INFO("BPM: %d at %02d:%02d:%02d", rollingBpm, hours, minutes, seconds);


  // if (bpm == 0) {
  //   lv_label_set_text_static(label_hr, "---");
  // } else {
  //   lv_label_set_text_fmt(label_hr, "%03d", bpm);
  // }
}