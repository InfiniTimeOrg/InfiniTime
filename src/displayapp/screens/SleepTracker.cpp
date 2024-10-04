#include "displayapp/screens/SleepTracker.h"

#include <lvgl/lvgl.h>
#include <components/heartrate/HeartRateController.h>
#include <components/datetime/DateTimeController.h>
#include <components/fs/FS.h>
#include <nrf_log.h>

using namespace Pinetime::Applications::Screens;

namespace {

  void BpmDataCallback(lv_task_t* task) {
    auto* screen = static_cast<SleepTracker*>(task->user_data);
    screen->GetBPM();
  }

}

SleepTracker::SleepTracker(Controllers::HeartRateController& heartRateController, Controllers::DateTime& dateTimeController, Controllers::FS& fsController, System::SystemTask& systemTask)
  : heartRateController {heartRateController}, dateTimeController {dateTimeController}, fsController {fsController}, wakeLock(systemTask) {

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

  // Write data to CSV
  int motion = 0; // Placeholder for motion data
  std::vector<std::tuple<int, int, int, int, int>> data = {std::make_tuple(hours, minutes, seconds, rollingBpm, motion)};
  WriteDataCSV("SleepTracker_Data.csv", data);

  // if (bpm == 0) {
  //   lv_label_set_text_static(label_hr, "---");
  // } else {
  //   lv_label_set_text_fmt(label_hr, "%03d", bpm);
  // }
}

// File IO Stuff

/*
* Write data to a CSV file
* Format: Time,BPM,Motion
*/
void SleepTracker::WriteDataCSV(const char* fileName, const std::vector<std::tuple<int, int, int, int, int>>& data) {
  lfs_file_t file;
  int err = fsController.FileOpen(&file, fileName, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND);
  if (err < 0) {
    // Handle error
    NRF_LOG_INFO("Error opening file: %d", err);
    return;
  }

  // Check if the file is empty
  // int fileSize = fsController.FileSeek(&file, 0);
  // if (fileSize == 0) {
  //   // Write header if file is empty
  //   const char* header = "Time,BPM,Motion\n";
  //   err = fsController.FileWrite(&file, reinterpret_cast<const uint8_t*>(header), strlen(header));
  //   if (err < 0) {
  //     // Handle error
  //     NRF_LOG_INFO("Error writing to file: %d", err);
  //     fsController.FileClose(&file);
  //     return;
  //   }
  // }

  // Write data
  for (const auto& entry : data) {
    int hours, minutes, seconds, bpm, motion;
    std::tie(hours, minutes, seconds, bpm, motion) = entry;
    char buffer[64];
    int len = snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d,%d,%d\n", hours, minutes, seconds, bpm, motion);
    int err = fsController.FileWrite(&file, reinterpret_cast<const uint8_t*>(buffer), len);
    if (err < 0) {
      // Handle error
      NRF_LOG_INFO("Error writing to file: %d", err);
      return;
    }
  }

  fsController.FileClose(&file);
}