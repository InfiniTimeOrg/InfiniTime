#include "displayapp/screens/SleepTracker.h"

#include <lvgl/lvgl.h>
#include <components/heartrate/HeartRateController.h>
#include <components/datetime/DateTimeController.h>
#include <components/fs/FS.h>
#include <nrf_log.h>

#include <sstream>
#include <numeric>

using namespace Pinetime::Applications::Screens;

namespace {

  void BpmDataCallback(lv_task_t* task) {
    auto* screen = static_cast<SleepTracker*>(task->user_data);
    screen->GetBPM();
  }

  void ClearDataCallback(lv_obj_t* btn, lv_event_t event) {
    if (event == LV_EVENT_CLICKED) {
      auto* screen = static_cast<SleepTracker*>(lv_obj_get_user_data(btn));
      screen->ClearDataCSV("SleepTracker_Data.csv");
    }
  }

  // void GetSleepInfoCallback(lv_obj_t* btn, lv_event_t event) {
  //   if (event == LV_EVENT_CLICKED) {
  //     auto* screen = static_cast<SleepTracker*>(lv_obj_get_user_data(btn));
  //     screen->GetSleepInfo(screen->ReadDataCSV("SleepTracker_Data.csv"));
  //   }
  // }

}

SleepTracker::SleepTracker(Controllers::HeartRateController& heartRateController, Controllers::DateTime& dateTimeController, Controllers::FS& fsController, System::SystemTask& systemTask)
  : heartRateController {heartRateController}, dateTimeController {dateTimeController}, fsController {fsController}, wakeLock(systemTask) {

  wakeLock.Lock();

  constexpr uint8_t btnWidth = 115;
  constexpr uint8_t btnHeight = 80;
  
  lv_obj_t* title = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text_static(title, "Sleep Tracker");
  lv_label_set_align(title, LV_LABEL_ALIGN_CENTER);
  lv_obj_align(title, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);

  label_hr = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_font(label_hr, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, &jetbrains_mono_76);

  // Create the refresh task
  mainRefreshTask = lv_task_create(RefreshTaskCallback, 100, LV_TASK_PRIO_MID, this);
  hrRefreshTask = lv_task_create(BpmDataCallback, 3000, LV_TASK_PRIO_MID, this);

  // Create the clear data button
  lv_obj_t* btnClear = lv_btn_create(lv_scr_act(), nullptr);
  btnClear->user_data = this;
  lv_obj_set_event_cb(btnClear, ClearDataCallback);
  lv_obj_set_size(btnClear, btnWidth, btnHeight);
  lv_obj_align(btnClear, lv_scr_act(), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
  lv_obj_t* txtClear = lv_label_create(btnClear, nullptr);
  lv_label_set_text(txtClear, "X");

  // Create the get info button
  // lv_obj_t* btnInfo = lv_btn_create(lv_scr_act(), nullptr);
  // btnInfo->user_data = this;
  // lv_obj_set_event_cb(btnInfo, GetSleepInfoCallback);
  // lv_obj_set_size(btnInfo, btnWidth, btnHeight);
  // lv_obj_align(btnInfo, lv_scr_act(), LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0);
  // lv_obj_t* txtInfo = lv_label_create(btnInfo, nullptr);
  // lv_label_set_text(txtInfo, "?");

  // const auto data = ReadDataCSV("SleepTracker_Data.csv");
  // for (const auto& entry : data) {
  //   int hours, minutes, seconds, bpm, motion;
  //   std::tie(hours, minutes, seconds, bpm, motion) = entry;
  //   NRF_LOG_INFO("Read data: %02d:%02d:%02d, %d, %d", hours, minutes, seconds, bpm, motion);
  // }
  // NRF_LOG_INFO("-------------------------------");
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

// Convert time to minutes
float SleepTracker::ConvertToMinutes(int hours, int minutes, int seconds) const {
  return hours * 60 + minutes + seconds / 60.0f;
}

// Get the moving average of BPM Values
// std::vector<float> SleepTracker::MovingAverage(const std::vector<int>& bpmData, int windowSize) const {
//   std::vector<float> smoothedBpm;
//   const int n = bpmData.size();
    
//   for (int i = 0; i < n - windowSize + 1; ++i) {
//     float sum = 0;
//     for (int j = 0; j < windowSize; ++j) {
//       sum += bpmData[i + j];
//     }
//     smoothedBpm.push_back(sum / windowSize);
//   }
    
//   return smoothedBpm;
// }

// Detect the sleep regions
// std::vector<std::pair<float, float>> SleepTracker::DetectSleepRegions(const std::vector<float>& bpmData, const std::vector<float>& time, float threshold) const {
//   std::vector<std::pair<float, float>> sleep_regions;
//   float start_time = -1;
//   bool in_sleep = false;
    
//   for (size_t i = 0; i < bpmData.size(); ++i) {
//     if (bpmData[i] < threshold) {
//       if (!in_sleep) {
//         start_time = time[i];  // Mark the start of sleep
//         in_sleep = true;
//       }
//     } else {
//       if (in_sleep) {
//         float end_time = time[i];  // Mark the end of sleep
//         sleep_regions.emplace_back(start_time, end_time);
//         in_sleep = false;
//       }
//     }
//   }
    
//   // In case the last region extends to the end of the data
//   if (in_sleep) {
//     sleep_regions.emplace_back(start_time, time.back());
//   }
    
//   return sleep_regions;
// }

// // Get Sleep Info
// void SleepTracker::GetSleepInfo(const std::vector<std::tuple<int, int, int, int, int>>& data) const {
//   std::vector<float> time;
//   std::vector<int> bpm;
    
//   // Extract the time (in minutes) and bpm from the data
//   for (const auto& entry : data) {
//     int hours, minutes, seconds, bpm_value, motion;
//     std::tie(hours, minutes, seconds, bpm_value, motion) = entry;
//     time.push_back(ConvertToMinutes(hours, minutes, seconds));
//     bpm.push_back(bpm_value);
//   }
  
//   // Compute the moving average with a window size of 5 (15 minutes smoothing, since each data point is 3 minutes)
//   const auto smoothed_bpm = MovingAverage(bpm, 5);
  
//   // Calculate a threshold as 80% of the average BPM
//   const float average_bpm = std::accumulate(bpm.begin(), bpm.end(), 0.0f) / bpm.size();
//   const float threshold = average_bpm * 0.8f;
  
//   // Detect multiple sleep regions
//   const auto sleep_regions = DetectSleepRegions(smoothed_bpm, time, threshold);
  
//   // Output sleep regions
//   if (!sleep_regions.empty()) {
//     for (const auto& region : sleep_regions) {
//       NRF_LOG_INFO("Sleep detected from %.2f minutes to %.2f minutes.", region.first, region.second);
//     }
//   } else {
//     NRF_LOG_INFO("No significant sleep regions detected.");
//   }

//   // Open the output file
//   lfs_file_t file;
//   int err = fsController.FileOpen(&file, "SleepTracker_SleepInfo.csv", LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC);
//   if (err < 0) {
//     // Handle error
//     NRF_LOG_INFO("Error opening file: %d", err);
//     return;
//   }

//   // Write sleep regions to the file
//   if (!sleep_regions.empty()) {
//     for (const auto& region : sleep_regions) {
//       char buffer[64];
//       int len = snprintf(buffer, sizeof(buffer), "Sleep detected from %.2f minutes to %.2f minutes.\n", region.first, region.second);
//       err = fsController.FileWrite(&file, reinterpret_cast<const uint8_t*>(buffer), len);
//       if (err < 0) {
//         // Handle error
//         NRF_LOG_INFO("Error writing to file: %d", err);
//         fsController.FileClose(&file);
//         return;
//       }
//     }
//   } else {
//     const char* noSleepMsg = "No significant sleep regions detected.\n";
//     err = fsController.FileWrite(&file, reinterpret_cast<const uint8_t*>(noSleepMsg), strlen(noSleepMsg));
//     if (err < 0) {
//       // Handle error
//       NRF_LOG_INFO("Error writing to file: %d", err);
//       fsController.FileClose(&file);
//       return;
//     }
//   }

//   // Close the file
//   fsController.FileClose(&file);
//   NRF_LOG_INFO("Sleep info written to SleepTracker_SleepInfo.csv");
// }

void SleepTracker::GetBPM() {
  // Get the heart rate from the controller
  prevBpm = bpm;
  bpm = heartRateController.HeartRate();

  if(prevBpm != 0)
    rollingBpm = (rollingBpm + bpm) / 2;
  else
    rollingBpm = bpm;

  // Get the current time from DateTimeController
  int hours = dateTimeController.Hours();
  int minutes = dateTimeController.Minutes();
  int seconds = dateTimeController.Seconds();

  // Log the BPM and current time
  NRF_LOG_INFO("BPM: %d at %02d:%02d:%02d", rollingBpm, hours, minutes, seconds);

  // Write data to CSV
  const int motion = 0; // Placeholder for motion data
  std::tuple<int, int, int, int, int> data[1] = {std::make_tuple(hours, minutes, seconds, bpm, motion)};
  WriteDataCSV("SleepTracker_Data.csv", data, 1);
}

// File IO Stuff

/*
* Write data to a CSV file
* Format: Time,BPM,Motion
*/
void SleepTracker::WriteDataCSV(const char* fileName, const std::tuple<int, int, int, int, int>* data, int dataSize) const {
  lfs_file_t file;
  int err = fsController.FileOpen(&file, fileName, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_APPEND);
  if (err < 0) {
    // Handle error
    NRF_LOG_INFO("Error opening file: %d", err);
    return;
  }

  for (int i = 0; i < dataSize; ++i) {
    int hours, minutes, seconds, bpm, motion;
    std::tie(hours, minutes, seconds, bpm, motion) = data[i];
    char buffer[64];
    int len = snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d,%d,%d\n", hours, minutes, seconds, bpm, motion);
    err = fsController.FileWrite(&file, reinterpret_cast<const uint8_t*>(buffer), len);
    if (err < 0) {
      // Handle error
      NRF_LOG_INFO("Error writing to file: %d", err);
      fsController.FileClose(&file);

      return;
    }
  }

  fsController.FileClose(&file);
}

// Read data from CSV
// std::vector<std::tuple<int, int, int, int, int>> SleepTracker::ReadDataCSV(const char* filename) const {
//   lfs_file_t file;
//   int err = fsController.FileOpen(&file, filename, LFS_O_RDONLY);
//   if (err < 0) {
//     // Handle error
//     NRF_LOG_INFO("Error opening file: %d", err);
//     return {};
//   }

//   std::vector<std::tuple<int, int, int, int, int>> data;
//   char buffer[128];
//   int bytesRead;

//   // Read data
//   while ((bytesRead = fsController.FileRead(&file, reinterpret_cast<uint8_t*>(buffer), sizeof(buffer))) > 0) {
//     std::istringstream dataStream(buffer);
//     std::string line;
//     while (std::getline(dataStream, line)) {
//       int hours, minutes, seconds, bpm, motion;
//       char colon1, colon2, comma1, comma2;
//       std::istringstream lineStream(line);
//       if (lineStream >> hours >> colon1 >> minutes >> colon2 >> seconds >> comma1 >> bpm >> comma2 >> motion) {
//         if (colon1 == ':' && colon2 == ':' && comma1 == ',' && comma2 == ',') {
//           data.emplace_back(hours, minutes, seconds, bpm, motion);
//         } else {
//           NRF_LOG_INFO("Parsing error: incorrect format in line: %s", line.c_str());
//         }
//       } else {
//         NRF_LOG_INFO("Parsing error: failed to parse line: %s", line.c_str());
//       }
//     }
//   }

//   fsController.FileClose(&file);
//   return data;
// }

// Clear data in CSV
void SleepTracker::ClearDataCSV(const char* filename) const {
  lfs_file_t file;
  int err = fsController.FileOpen(&file, filename, LFS_O_WRONLY | LFS_O_TRUNC);
  if (err < 0) {
    // Handle error
    NRF_LOG_INFO("Error opening file: %d", err);
    return;
  }

  fsController.FileClose(&file);
  NRF_LOG_INFO("CSV data cleared");
}