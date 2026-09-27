#include "components/sleep/SleepTracker.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include "components/fs/FS.h"

using namespace Pinetime::Controllers;

namespace {
  constexpr uint32_t sameNightSeconds = 3 * 60 * 60;
  constexpr float radiansToDegrees = 57.2957795f;

  uint32_t Distance(uint32_t a, uint32_t b) {
    return a > b ? a - b : b - a;
  }
}

SleepTracker::SleepTracker(FS& fs) : fs {fs} {
  mutex = xSemaphoreCreateMutex();
  xSemaphoreGive(mutex);
  medians.fill(Sleep::Invalid);
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
    deltaCount = 0;
    minutePostureChange = false;
    minuteOffWrist = false;
    minuteHeartRate = 0;
    minuteHasSamples = false;
  }

  // Range of each axis during this minute, for non-wear detection
  const std::array<int16_t, 3> sample {x, y, z};
  for (size_t axis = 0; axis < 3; axis++) {
    minuteMin[axis] = minuteHasSamples ? std::min(minuteMin[axis], sample[axis]) : sample[axis];
    minuteMax[axis] = minuteHasSamples ? std::max(minuteMax[axis], sample[axis]) : sample[axis];
  }
  minuteHasSamples = true;
  minuteOffWrist = minuteOffWrist || offWrist;
  if (heartRate != 0) {
    minuteHeartRate = heartRate;
  }

  // Forearm angle from the 5 second average, like GGIR's anglez
  sumX += x;
  sumY += y;
  sumZ += z;
  if (++samples < SamplesPerEpoch) {
    return;
  }
  const float meanX = static_cast<float>(sumX) / SamplesPerEpoch;
  const float meanY = static_cast<float>(sumY) / SamplesPerEpoch;
  const float meanZ = static_cast<float>(sumZ) / SamplesPerEpoch;
  sumX = sumY = sumZ = 0;
  samples = 0;

  const float angle = std::atan2(meanZ, std::sqrt(meanX * meanX + meanY * meanY)) * radiansToDegrees;
  if (haveAngle) {
    const float change = std::fabs(angle - previousAngle);
    if (change > Sleep::PostureChangeDegrees) {
      minutePostureChange = true;
    }
    if (deltaCount < EpochsPerMinute) {
      deltas[deltaCount++] = static_cast<uint8_t>(std::min(std::lround(change / Sleep::DeltaUnit), static_cast<long>(Sleep::DeltaMax)));
    }
  }
  previousAngle = angle;
  haveAngle = true;
}

void SleepTracker::Push(uint8_t median, bool postureChange) {
  newest = (newest + 1) % Minutes;
  medians[newest] = median;
  Sleep::SetBit(changeBits.data(), newest, postureChange);
  count = std::min<uint16_t>(count + 1, Minutes);
}

void SleepTracker::MarkInvalid(uint16_t minutes) {
  for (uint16_t i = 0; i < minutes && i < count; i++) {
    medians[(newest + Minutes - i) % Minutes] = Sleep::Invalid;
  }
}

void SleepTracker::CloseMinute() {
  uint8_t median = Sleep::Invalid;
  if (!minuteOffWrist && deltaCount > 0) {
    std::nth_element(deltas.begin(), deltas.begin() + deltaCount / 2, deltas.begin() + deltaCount);
    median = deltas[deltaCount / 2];
  }

  // Non-wear (GGIR): range under 50mg on at least 2 of the 3 axes for 60 minutes
  if (minuteHasSamples) {
    auto quietAxes = [&]() {
      uint8_t quiet = 0;
      for (size_t axis = 0; axis < 3; axis++) {
        quiet += (stillMax[axis] - stillMin[axis]) < NonWearRange ? 1 : 0;
      }
      return quiet;
    };
    if (stillMinutes > 0) {
      for (size_t axis = 0; axis < 3; axis++) {
        stillMin[axis] = std::min(stillMin[axis], minuteMin[axis]);
        stillMax[axis] = std::max(stillMax[axis], minuteMax[axis]);
      }
    }
    if (stillMinutes == 0 || quietAxes() < 2) {
      // Start a new still stretch with this minute
      stillMin = minuteMin;
      stillMax = minuteMax;
      stillMinutes = quietAxes() >= 2 ? 1 : 0;
    } else {
      stillMinutes++;
    }
  } else {
    stillMinutes = 0;
  }

  Push(median, minutePostureChange);
  if (stillMinutes == NonWearMinutes) {
    MarkInvalid(NonWearMinutes);
  } else if (stillMinutes > NonWearMinutes) {
    medians[newest] = Sleep::Invalid;
  }

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
      Push(Sleep::Invalid, false);
      stillMinutes = 0;
    }
  }
  currentMinute = minute;
}

Pinetime::Controllers::Sleep::Window SleepTracker::MakeWindow() const {
  return Sleep::Window(medians.data(), changeBits.data(), Minutes, newest, count);
}

Pinetime::Controllers::Sleep::Scratch SleepTracker::MakeScratch() {
  return {windowBits.data(), asleepBits.data(), histogram.data()};
}

void SleepTracker::Build(const Sleep::Session& session, Night& night) {
  const Sleep::Window window = MakeWindow();
  const Sleep::Scratch scratch = MakeScratch();
  // Window index i is the minute (currentMinute - count + i)
  const uint32_t firstMinute = currentMinute - count;

  night = {};
  night.start = (firstMinute + session.start) * 60;
  night.inBed = session.end - session.start + 1;
  night.asleep = session.asleep;
  night.awake = session.awake;
  night.wakeUps = session.wakeUps;
  night.ongoing = session.ongoing ? 1 : 0;
  night.epochs = std::min<uint16_t>((night.inBed + EpochMinutes - 1) / EpochMinutes, MaxEpochs);

  for (uint8_t epoch = 0; epoch < night.epochs; epoch++) {
    uint8_t awake = 0;
    uint8_t asleep = 0;
    for (uint8_t m = 0; m < EpochMinutes; m++) {
      const uint16_t index = session.start + epoch * EpochMinutes + m;
      switch (Sleep::StageAt(window, scratch, session, index)) {
        case Sleep::Stage::Awake:
          awake++;
          break;
        case Sleep::Stage::Asleep:
          asleep++;
          break;
        case Sleep::Stage::None:
          break;
      }
    }
    Sleep::Stage stage = Sleep::Stage::None;
    if (awake > asleep) {
      stage = Sleep::Stage::Awake;
    } else if (asleep > 0) {
      stage = Sleep::Stage::Asleep;
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
}

bool SleepTracker::Live(Night& night) {
  xSemaphoreTake(mutex, portMAX_DELAY);
  const Sleep::Session session = Sleep::Analyze(MakeWindow(), MakeScratch());
  if (session.found) {
    Build(session, night);
  }
  xSemaphoreGive(mutex);
  return session.found;
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
