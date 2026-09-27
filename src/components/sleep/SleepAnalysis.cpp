#include "components/sleep/SleepAnalysis.h"

#include <algorithm>

using namespace Pinetime::Controllers::Sleep;

namespace {
  // Median of the valid minutes in a window of RollingMinutes around index, Invalid if too few
  uint8_t RollingMedian(const Window& window, uint16_t index) {
    uint8_t values[RollingMinutes];
    uint8_t n = 0;
    const int32_t half = RollingMinutes / 2;
    for (int32_t i = static_cast<int32_t>(index) - half; i <= static_cast<int32_t>(index) + half; i++) {
      if (i >= 0 && i < window.Size() && window.IsValid(i)) {
        values[n++] = window.Median(i);
      }
    }
    if (n <= RollingMinutes / 2) {
      return Invalid;
    }
    // Insertion sort, at most RollingMinutes values
    for (uint8_t i = 1; i < n; i++) {
      const uint8_t value = values[i];
      uint8_t j = i;
      while (j > 0 && values[j - 1] > value) {
        values[j] = values[j - 1];
        j--;
      }
      values[j] = value;
    }
    return values[n / 2];
  }

  // Length of the run of equal bits starting at index
  uint16_t Run(const uint8_t* bits, uint16_t size, uint16_t index) {
    const bool value = GetBit(bits, index);
    uint16_t i = index;
    while (i < size && GetBit(bits, i) == value) {
      i++;
    }
    return i - index;
  }

  void Fill(uint8_t* bits, uint16_t from, uint16_t length, bool value) {
    for (uint16_t i = from; i < from + length; i++) {
      SetBit(bits, i, value);
    }
  }
}

Session Pinetime::Controllers::Sleep::Analyze(const Window& window, const Scratch& scratch) {
  const uint16_t size = window.Size();
  Session session;
  if (size == 0) {
    return session;
  }

  // HDCZA threshold: 10th percentile of the rolling median, times 15, kept within 0.13-0.5 degrees
  std::fill(scratch.histogram, scratch.histogram + 256, 0);
  uint32_t total = 0;
  for (uint16_t i = 0; i < size; i++) {
    const uint8_t value = RollingMedian(window, i);
    if (value != Invalid) {
      scratch.histogram[value]++;
      total++;
    }
  }
  if (total == 0) {
    return session;
  }
  const uint32_t rank = (total * ThresholdPercentile + 99) / 100;
  uint32_t seen = 0;
  uint16_t percentile = 0;
  for (uint16_t v = 0; v < 256; v++) {
    seen += scratch.histogram[v];
    if (seen >= rank) {
      percentile = v;
      break;
    }
  }
  const uint16_t threshold = std::clamp<uint16_t>(percentile * ThresholdMultiplier, ThresholdMin, ThresholdMax);
  session.threshold = threshold;

  // Candidate minutes: rolling median below the threshold
  uint8_t* bits = scratch.windowBits;
  for (uint16_t i = 0; i < size; i++) {
    const uint8_t value = RollingMedian(window, i);
    SetBit(bits, i, value != Invalid && value < threshold);
  }

  // Drop blocks of MinBlockMinutes or less
  for (uint16_t i = 0; i < size;) {
    const uint16_t run = Run(bits, size, i);
    if (GetBit(bits, i) && run <= MinBlockMinutes) {
      Fill(bits, i, run, false);
    }
    i += run;
  }

  // Fill gaps shorter than MaxGapMinutes between blocks
  for (uint16_t i = 0; i < size;) {
    const uint16_t run = Run(bits, size, i);
    if (!GetBit(bits, i) && run < MaxGapMinutes && i > 0 && i + run < size) {
      Fill(bits, i, run, true);
    }
    i += run;
  }

  // The longest block is the sleep period (the latest one on a tie)
  uint16_t bestStart = 0;
  uint16_t bestLength = 0;
  for (uint16_t i = 0; i < size;) {
    const uint16_t run = Run(bits, size, i);
    if (GetBit(bits, i) && run >= bestLength) {
      bestStart = i;
      bestLength = run;
    }
    i += run;
  }
  if (bestLength == 0) {
    return session;
  }
  session.start = bestStart;
  session.end = bestStart + bestLength - 1;

  // Sustained inactivity bouts: stretches without posture changes of more than 5 minutes,
  // including the minutes with the posture changes that bound them
  uint8_t* asleep = scratch.asleepBits;
  std::fill(asleep, asleep + size / 8 + 1, 0);
  uint16_t runStart = 0;
  uint16_t runLength = 0;
  for (uint16_t i = 0; i <= size; i++) {
    const bool quiet = i < size && window.IsValid(i) && !window.PostureChange(i);
    if (quiet) {
      if (runLength == 0) {
        runStart = i;
      }
      runLength++;
      continue;
    }
    if (runLength >= SustainedInactivityMinutes) {
      const uint16_t from = (runStart > 0 && window.IsValid(runStart - 1)) ? runStart - 1 : runStart;
      const uint16_t to = (i < size && window.IsValid(i)) ? i : i - 1;
      Fill(asleep, from, to - from + 1, true);
    }
    runLength = 0;
  }

  uint16_t awakeRun = 0;
  for (uint16_t i = session.start; i <= session.end; i++) {
    switch (StageAt(window, scratch, session, i)) {
      case Stage::Asleep:
        session.asleep++;
        awakeRun = 0;
        break;
      case Stage::Awake:
        session.awake++;
        if (++awakeRun == WakeUpMinutes && session.wakeUps < 255) {
          session.wakeUps++;
        }
        break;
      case Stage::None:
        break;
    }
  }

  session.found = session.asleep >= MinAsleepMinutes;
  session.ongoing = (size - 1 - session.end) <= OngoingMinutes;
  return session;
}

Stage Pinetime::Controllers::Sleep::StageAt(const Window& window, const Scratch& scratch, const Session& session, uint16_t index) {
  if (index < session.start || index > session.end || !window.IsValid(index)) {
    return Stage::None;
  }
  return GetBit(scratch.asleepBits, index) ? Stage::Asleep : Stage::Awake;
}
