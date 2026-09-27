#include "components/sleep/SleepAnalysis.h"

using namespace Pinetime::Controllers::Sleep;

namespace {
  bool IsValid(uint8_t level) {
    return level <= LevelMax;
  }

  bool ScoreAsleep(const Window& window, uint16_t index) {
    const uint8_t own = window[index];
    if (!IsValid(own)) {
      return false;
    }
    uint8_t active = 0;
    const int32_t from = static_cast<int32_t>(index) - WindowBefore;
    const int32_t to = static_cast<int32_t>(index) + WindowAfter;
    for (int32_t i = from; i <= to; i++) {
      if (i < 0 || i >= window.Size()) {
        continue;
      }
      const uint8_t level = window[i];
      if (!IsValid(level) || level > ActiveLevel) {
        active++;
      }
    }
    return active <= MaxActiveInWindow;
  }

  // Count of consecutive minutes with the same asleep value starting at index, going forward or backward
  uint16_t RunLength(const uint8_t* bits, uint16_t size, uint16_t index, bool value, int8_t direction) {
    uint16_t length = 0;
    int32_t i = index;
    while (i >= 0 && i < size && GetBit(bits, i) == value) {
      length++;
      i += direction;
    }
    return length;
  }
}

Stage Pinetime::Controllers::Sleep::StageAt(const Window& window, const uint8_t* asleepBits, uint16_t index) {
  if (!IsValid(window[index])) {
    return Stage::None;
  }
  if (!GetBit(asleepBits, index)) {
    return Stage::Awake;
  }
  const int32_t from = static_cast<int32_t>(index) - StillRadius;
  const int32_t to = static_cast<int32_t>(index) + StillRadius;
  for (int32_t i = from; i <= to; i++) {
    if (i < 0 || i >= window.Size()) {
      continue;
    }
    if (window[i] != 0) {
      return Stage::Light;
    }
  }
  return Stage::Still;
}

Session Pinetime::Controllers::Sleep::Analyze(const Window& window, uint8_t* asleepBits) {
  const uint16_t size = window.Size();

  for (uint16_t i = 0; i < size; i++) {
    SetBit(asleepBits, i, ScoreAsleep(window, i));
  }

  // Rescore brief arousals (1-2 minutes) inside solid sleep as sleep, a turn-over shouldn't count as waking up
  for (uint16_t i = 0; i < size; i++) {
    if (GetBit(asleepBits, i) || !IsValid(window[i])) {
      continue;
    }
    const uint16_t awakeRun = RunLength(asleepBits, size, i, false, 1);
    const uint16_t end = i + awakeRun;
    if (awakeRun <= 2 && i > 0 && end < size && RunLength(asleepBits, size, i - 1, true, -1) >= OnsetMinutes &&
        RunLength(asleepBits, size, end, true, 1) >= OnsetMinutes) {
      bool allValid = true;
      for (uint16_t j = i; j < end; j++) {
        allValid = allValid && IsValid(window[j]);
      }
      if (allValid) {
        for (uint16_t j = i; j < end; j++) {
          SetBit(asleepBits, j, true);
        }
      }
    }
    i = end;
  }

  Session best;
  bool inBlock = false;
  uint16_t blockStart = 0;
  uint16_t lastAsleep = 0;
  uint16_t asleepCount = 0;
  uint16_t run = 0;

  auto closeBlock = [&](bool atEnd) {
    // Asleep at least half of the time in bed, otherwise it's restless noise rather than a night
    const bool efficient = asleepCount * 2 >= (lastAsleep - blockStart + 1);
    if (inBlock && efficient && asleepCount >= MinAsleepMinutes && asleepCount >= best.asleep) {
      best.found = true;
      best.start = blockStart;
      best.end = lastAsleep;
      best.asleep = asleepCount;
      best.ongoing = atEnd && (size - 1 - lastAsleep) <= OngoingMinutes;
    }
    inBlock = false;
    asleepCount = 0;
  };

  for (uint16_t i = 0; i < size; i++) {
    const bool valid = IsValid(window[i]);
    const bool asleep = valid && GetBit(asleepBits, i);
    run = asleep ? run + 1 : 0;

    if (inBlock) {
      // Off-wrist minutes don't end the session by themselves (e.g. a short charge), they count as unknown
      if ((i - lastAsleep) > MaxWakeGapMinutes) {
        closeBlock(false);
      } else if (asleep) {
        lastAsleep = i;
        asleepCount++;
      }
    }
    if (!inBlock && run >= OnsetMinutes) {
      inBlock = true;
      blockStart = i + 1 - OnsetMinutes;
      lastAsleep = i;
      asleepCount = OnsetMinutes;
    }
  }
  closeBlock(true);

  if (best.found) {
    best.asleep = 0;
    for (uint16_t i = best.start; i <= best.end; i++) {
      switch (StageAt(window, asleepBits, i)) {
        case Stage::Still:
          best.still++;
          best.asleep++;
          break;
        case Stage::Light:
          best.light++;
          best.asleep++;
          break;
        case Stage::Awake:
          best.awake++;
          break;
        case Stage::None:
          break;
      }
    }
  }
  return best;
}
