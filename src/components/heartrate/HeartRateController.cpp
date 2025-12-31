#include "components/heartrate/HeartRateController.h"
#include <heartratetask/HeartRateTask.h>
#include <systemtask/SystemTask.h>

using namespace Pinetime::Controllers;

HeartRateZoneSettings::HeartRateZoneSettings() {
  version = 0;
  adjustMsDelay = 300000;
  exerciseMsTarget = 1800000;
  age = 25;
  maxHeartRate = maxHeartRateEstimate(age);
  percentTarget = {50, 60, 70, 80, 90};
  bpmTarget = bpmZones(percentTarget, maxHeartRate);
  allowCalibration = true;
};

HeartRateController::HeartRateController(Pinetime::Controllers::FS& fs) : fs {fs} {
  LoadSettingsFromFile();
};

void HeartRateController::Update(HeartRateController::States newState, uint8_t heartRate) {
  this->state = newState;
  if (this->heartRate != heartRate) {
    uint32_t ts = xTaskGetTickCount();
    uint32_t z;
    zone = 0;
    auto adjustMax = pdMS_TO_TICKS(this->zSettings.adjustMsDelay);
    for (z = this->zSettings.bpmTarget.size() - 1; z < this->zSettings.bpmTarget.size(); --z) {
      if (this->heartRate >= this->zSettings.bpmTarget[z]) {
        uint32_t dt = ts - lastActiveTime;
        currentActivity.zoneTime[z] += dt;
        zone = z + 1;
        // don't make increases unless this is consistantly higher than normal (zone 5 is max)
        if (this->zSettings.allowCalibration && zone >= 5 && dt > adjustMax) {
          this->zSettings.maxHeartRate = this->zSettings.maxHeartRate >= this->heartRate ? this->zSettings.maxHeartRate : this->heartRate;
          this->zSettings.bpmTarget = bpmZones(this->zSettings.percentTarget, this->zSettings.maxHeartRate);
        }
        break;
      }
    }
    lastActiveTime = ts;

    this->heartRate = heartRate;

    service->OnNewHeartRateValue(heartRate);
  }
}

void HeartRateController::AdvanceDay() {
  HeartRateZones<uint16_t> convertedActivity {};
  auto ticksPerUnit = pdMS_TO_TICKS(2000); // 2s

  auto totalTime = currentActivity.totalTime();
  for (uint32_t i = 0; i < convertedActivity.zoneTime.size(); i++) {
    convertedActivity.zoneTime[i] = (uint16_t)Utility::RoundedDiv((uint32_t)totalTime, (uint32_t)ticksPerUnit);
  }

  activity[0] = convertedActivity;
  activity++;
}

void HeartRateController::SaveSettingsToFile() const {
  lfs_dir systemDir;
  if (fs.DirOpen("/.system", &systemDir) != LFS_ERR_OK) {
    fs.DirCreate("/.system");
  }
  fs.DirClose(&systemDir);
  lfs_file_t heartRateZoneFile;
  if (fs.FileOpen(&heartRateZoneFile, "/.system/hrzs.dat", LFS_O_WRONLY | LFS_O_CREAT) != LFS_ERR_OK) {
    NRF_LOG_WARNING("[HeartRateController] Failed to open heart rate zone settings file for saving");
    return;
  }

  fs.FileWrite(&heartRateZoneFile, reinterpret_cast<const uint8_t*>(&(this->zSettings)), sizeof(this->zSettings));
  fs.FileClose(&heartRateZoneFile);
  NRF_LOG_INFO("[HeartRateController] Saved heart rate zone settings with format version %u to file", this->zSettings.version);

  lfs_file_t zoneDataFile;
  if (fs.FileOpen(&zoneDataFile, "/.system/hrz.dat", LFS_O_WRONLY | LFS_O_CREAT) != LFS_ERR_OK) {
    NRF_LOG_WARNING("[HeartRateController] Failed to open heart rate zone data file for saving");
    return;
  }

  // Version 1: Raw Data Dump to File
  const int version = 0x1;
  fs.FileWrite(&zoneDataFile, reinterpret_cast<const uint8_t*>(&version), sizeof(int));
  // Raw Data
  fs.FileWrite(&zoneDataFile, reinterpret_cast<const uint8_t*>(&activity), sizeof(activity));
  fs.FileClose(&zoneDataFile);
  NRF_LOG_INFO("[HeartRateController] Saved heart rate zone data with format version %u to file", this->zSettings.version);
}

void HeartRateController::LoadSettingsFromFile() {
  HeartRateZoneSettings HRZSettings;
  lfs_file_t settingsFile;

  if (fs.FileOpen(&settingsFile, "/.system/hrzs.dat", LFS_O_RDONLY) != LFS_ERR_OK) {

  } else {
    fs.FileRead(&settingsFile, reinterpret_cast<uint8_t*>(&HRZSettings), sizeof(HRZSettings));
    fs.FileClose(&settingsFile);
    if (HRZSettings.version == zSettings.version) {
      zSettings = HRZSettings;
    }
  }

  lfs_file_t dataFile;

  if (fs.FileOpen(&dataFile, "/.system/hrz.dat", LFS_O_RDONLY) != LFS_ERR_OK) {

  } else {
    int version = 0;
    decltype(activity) HRZdata;
    fs.FileRead(&dataFile, reinterpret_cast<uint8_t*>(&version), sizeof(version));
    fs.FileRead(&dataFile, reinterpret_cast<uint8_t*>(&HRZdata), sizeof(HRZdata));
    fs.FileClose(&dataFile);
    if (version == 0x1) {
      activity = HRZdata;
    }
  }
}

void HeartRateController::Enable() {
  if (task != nullptr) {
    state = States::NotEnoughData;
    task->PushMessage(Pinetime::Applications::HeartRateTask::Messages::Enable);
  }
}

void HeartRateController::Disable() {
  if (task != nullptr) {
    state = States::Stopped;
    task->PushMessage(Pinetime::Applications::HeartRateTask::Messages::Disable);
  }
}

void HeartRateController::SetHeartRateTask(Pinetime::Applications::HeartRateTask* task) {
  this->task = task;
}

void HeartRateController::SetService(Pinetime::Controllers::HeartRateService* service) {
  this->service = service;
}
