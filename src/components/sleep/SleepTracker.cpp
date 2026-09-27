#include "components/sleep/SleepTracker.h"

#include <algorithm>
#include <cstdlib>
#include "components/fs/FS.h"

using namespace Pinetime::Controllers;

namespace {
  constexpr uint32_t sameNightSeconds = 3 * 60 * 60;

  uint32_t Distance(uint32_t a, uint32_t b) {
    return a > b ? a - b : b - a;
  }
}

SleepTracker::SleepTracker(FS& fs) : fs {fs} {
  mutex = xSemaphoreCreateMutex();
  xSemaphoreGive(mutex);
  levels.fill(Sleep::LevelNoData);
}

void SleepTracker::Load() {
  lfs_file_t file;
  if (fs.FileOpen(&file, fileName, LFS_O_RDONLY) != LFS_ERR_OK) {
    return;
  }
  uint32_t header[2] = {};
  bool ok = fs.FileRead(&file, reinterpret_cast<uint8_t*>(header), sizeof(header)) == sizeof(header);
  const uint8_t version = header[1] & 0xFF;
  const uint8_t storedCount = (header[1] >> 8) & 0xFF;
  ok = ok && header[0] == magic && version == fileVersion && storedCount <= MaxNights;
  if (ok) {
    ok = fs.FileRead(&file, reinterpret_cast<uint8_t*>(history.data()), sizeof(history)) == sizeof(history);
  }
  fs.FileClose(&file);
  historyCount = ok ? storedCount : 0;
}

void SleepTracker::Save() {
  xSemaphoreTake(mutex, portMAX_DELAY);
  dirty = false;
  lfs_file_t file;
  if (fs.FileOpen(&file, fileName, LFS_O_WRONLY | LFS_O_CREAT | LFS_O_TRUNC) == LFS_ERR_OK) {
    const uint32_t header[2] = {magic, static_cast<uint32_t>(fileVersion) | (static_cast<uint32_t>(historyCount) << 8)};
    fs.FileWrite(&file, reinterpret_cast<const uint8_t*>(header), sizeof(header));
    fs.FileWrite(&file, reinterpret_cast<const uint8_t*>(history.data()), sizeof(history));
    fs.FileClose(&file);
  } else {
    dirty = true;
  }
  xSemaphoreGive(mutex);
}

void SleepTracker::OnMotion(int16_t x, int16_t y, int16_t z, uint32_t localMinute, bool offWrist, uint8_t heartRate) {
  if (!started) {
    started = true;
    currentMinute = localMinute;
  } else if (localMinute != currentMinute) {
    xSemaphoreTake(mutex, portMAX_DELAY);
    if (localMinute > currentMinute && localMinute - currentMinute <= Minutes) {
      CloseMinute();
      AdvanceTo(localMinute);
    } else {
      // The clock was set backwards or jumped a long way: start over
      count = 0;
      stillMinutes = 0;
      currentMinute = localMinute;
    }
    xSemaphoreGive(mutex);
    movement = 0;
    minuteOffWrist = false;
    minuteHeartRate = 0;
  }

  if (havePrevious) {
    const int32_t change = std::abs(x - previousX) + std::abs(y - previousY) + std::abs(z - previousZ);
    if (change > NoiseFloor) {
      movement += change - NoiseFloor;
    }
  }
  havePrevious = true;
  previousX = x;
  previousY = y;
  previousZ = z;

  minuteOffWrist = minuteOffWrist || offWrist;
  if (heartRate != 0) {
    minuteHeartRate = heartRate;
  }
}

void SleepTracker::CloseMinute() {
  uint8_t level = static_cast<uint8_t>(std::min<uint32_t>(movement / LevelDivider, Sleep::LevelMax));
  if (minuteOffWrist) {
    level = Sleep::LevelOffWrist;
  }

  // Nobody lies perfectly still for hours: the watch is on the nightstand
  stillMinutes = (level == 0) ? stillMinutes + 1 : 0;
  if (stillMinutes >= OffWristStillMinutes) {
    if (stillMinutes == OffWristStillMinutes) {
      for (uint16_t i = 0; i < OffWristStillMinutes - 1 && i < count; i++) {
        levels[(newest + Minutes - i) % Minutes] = Sleep::LevelOffWrist;
      }
    }
    level = Sleep::LevelOffWrist;
  }

  newest = (newest + 1) % Minutes;
  levels[newest] = level;
  count = std::min<uint16_t>(count + 1, Minutes);

  if (minuteHeartRate != 0) {
    auto& slot = heartRates[(currentMinute / 10) % HeartRateSlots];
    slot.sum += minuteHeartRate;
    slot.samples++;
  }
}

void SleepTracker::AdvanceTo(uint32_t minute) {
  for (uint32_t m = currentMinute + 1; m <= minute; m++) {
    if (m % 10 == 0) {
      heartRates[(m / 10) % HeartRateSlots] = {};
    }
    if (m != minute) {
      // Minutes without samples (shouldn't happen, the system task runs all the time)
      newest = (newest + 1) % Minutes;
      levels[newest] = Sleep::LevelNoData;
      count = std::min<uint16_t>(count + 1, Minutes);
      stillMinutes = 0;
    }
  }
  currentMinute = minute;
}

bool SleepTracker::Build(const Sleep::Session& session, Night& night) {
  const Sleep::Window window(levels.data(), Minutes, newest, count);
  // Window index i is the minute (currentMinute - count + i)
  const uint32_t firstMinute = currentMinute - count;

  night = {};
  night.start = (firstMinute + session.start) * 60;
  night.inBed = session.end - session.start + 1;
  night.asleep = session.asleep;
  night.still = session.still;
  night.awake = session.awake;
  night.ongoing = session.ongoing ? 1 : 0;
  night.epochs = std::min<uint16_t>((night.inBed + EpochMinutes - 1) / EpochMinutes, MaxEpochs);

  for (uint8_t epoch = 0; epoch < night.epochs; epoch++) {
    uint8_t awake = 0;
    uint8_t still = 0;
    uint8_t light = 0;
    for (uint8_t m = 0; m < EpochMinutes; m++) {
      const uint16_t index = session.start + epoch * EpochMinutes + m;
      if (index > session.end) {
        break;
      }
      switch (Sleep::StageAt(window, asleepBits.data(), index)) {
        case Sleep::Stage::Awake:
          awake++;
          break;
        case Sleep::Stage::Still:
          still++;
          break;
        case Sleep::Stage::Light:
          light++;
          break;
        case Sleep::Stage::None:
          break;
      }
    }
    Sleep::Stage stage = Sleep::Stage::None;
    if (awake >= 2) {
      stage = Sleep::Stage::Awake;
    } else if (still >= 3) {
      stage = Sleep::Stage::Still;
    } else if (still + light > 0) {
      stage = Sleep::Stage::Light;
    }
    night.stages[epoch / 4] |= static_cast<uint8_t>(stage) << ((epoch % 4) * 2);
  }

  uint32_t sum = 0;
  uint8_t slots = 0;
  uint8_t minimum = 255;
  const uint32_t from = (firstMinute + session.start) / 10;
  const uint32_t to = (firstMinute + session.end) / 10;
  for (uint32_t s = from; s <= to && s - from < HeartRateSlots; s++) {
    const auto& slot = heartRates[s % HeartRateSlots];
    if (slot.samples == 0) {
      continue;
    }
    const uint8_t average = slot.sum / slot.samples;
    sum += average;
    slots++;
    minimum = std::min(minimum, average);
  }
  if (slots > 0) {
    night.avgHeartRate = sum / slots;
    night.minHeartRate = minimum;
  }
  return true;
}

bool SleepTracker::Live(Night& night) {
  xSemaphoreTake(mutex, portMAX_DELAY);
  const Sleep::Window window(levels.data(), Minutes, newest, count);
  const Sleep::Session session = Sleep::Analyze(window, asleepBits.data());
  const bool found = session.found && Build(session, night);
  xSemaphoreGive(mutex);
  return found;
}

void SleepTracker::Finalize() {
  Night night;
  if (!Live(night) || night.ongoing != 0) {
    return;
  }
  xSemaphoreTake(mutex, portMAX_DELAY);
  if (historyCount > 0 && Distance(history[0].start, night.start) < sameNightSeconds) {
    // Same night seen again. Only take it if it grew: once the start scrolls out of the
    // 24h window the analysis sees a truncated night that must not replace the full one.
    if (night.inBed > history[0].inBed) {
      history[0] = night;
      dirty = true;
    }
  } else if (historyCount == 0 || night.start > history[0].start) {
    for (size_t i = MaxNights - 1; i > 0; i--) {
      history[i] = history[i - 1];
    }
    history[0] = night;
    historyCount = std::min<uint8_t>(historyCount + 1, MaxNights);
    dirty = true;
  }
  xSemaphoreGive(mutex);
}

SleepTracker::Night SleepTracker::History(size_t index) const {
  if (index >= historyCount) {
    return {};
  }
  return history[index];
}
