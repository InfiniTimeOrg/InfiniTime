#pragma once

#include <cstddef>
#include <cstdint>

// Sleep detection from wrist accelerometer data, following the open GGIR methods:
//
// - Sleep period time window: HDCZA (van Hees et al. 2018, Scientific Reports 8:12975).
//   The 5 minute rolling median of the absolute change in forearm angle is compared to a
//   threshold derived from the person's own data (10th percentile x 15, kept within
//   0.13-0.5 degrees). Still blocks of 30 minutes or less are dropped, gaps under 60 minutes
//   are filled and the longest block is the night.
// - Sleep inside that window: sustained inactivity bouts (van Hees et al. 2015, PLoS ONE
//   10:e0142533). Asleep when the forearm angle doesn't change by more than 5 degrees for
//   more than 5 minutes, awake otherwise.
//
// The inputs are per minute: the median absolute change of the forearm angle between
// consecutive 5 second epochs, and whether any change exceeded 5 degrees (a posture change).
// Pure functions without FreeRTOS dependencies so they can be tested on a PC.
namespace Pinetime {
  namespace Controllers {
    namespace Sleep {
      static constexpr float DeltaUnit = 0.02f; // Degrees per unit of the stored median angle change
      static constexpr uint8_t DeltaMax = 254;  // Stored values are capped here
      static constexpr uint8_t Invalid = 255;   // Watch not worn or no data for that minute
      static constexpr float PostureChangeDegrees = 5.0f;

      // GGIR defaults
      static constexpr uint8_t RollingMinutes = 5;
      static constexpr uint8_t ThresholdPercentile = 10;
      static constexpr uint8_t ThresholdMultiplier = 15;
      static constexpr uint8_t ThresholdMin = 7;  // 0.13 degrees, rounded up to the stored resolution
      static constexpr uint8_t ThresholdMax = 25; // 0.50 degrees
      static constexpr uint16_t MinBlockMinutes = 30;
      static constexpr uint16_t MaxGapMinutes = 60;
      static constexpr uint8_t SustainedInactivityMinutes = 5;

      // Ours
      static constexpr uint16_t MinAsleepMinutes = 60; // Shorter nights (naps) are ignored
      static constexpr uint8_t OngoingMinutes = 5;     // Window ending this close to now is still in progress
      static constexpr uint8_t WakeUpMinutes = 5;      // Awake stretches at least this long count as a wake-up

      enum class Stage : uint8_t { None = 0, Awake = 1, Asleep = 2 };

      inline bool GetBit(const uint8_t* bits, uint16_t index) {
        return (bits[index / 8] & (1 << (index % 8))) != 0;
      }

      inline void SetBit(uint8_t* bits, uint16_t index, bool value) {
        if (value) {
          bits[index / 8] |= (1 << (index % 8));
        } else {
          bits[index / 8] &= ~(1 << (index % 8));
        }
      }

      // Chronological view over ring buffers: index 0 is the oldest minute, Size() - 1 the newest
      class Window {
      public:
        Window(const uint8_t* medians, const uint8_t* changeBits, uint16_t capacity, uint16_t newest, uint16_t count)
          : medians {medians}, changeBits {changeBits}, capacity {capacity}, newest {newest}, count {count} {
        }

        uint16_t Size() const {
          return count;
        }

        // Median absolute angle change in DeltaUnits, or Invalid
        uint8_t Median(uint16_t index) const {
          return medians[Slot(index)];
        }

        bool IsValid(uint16_t index) const {
          return Median(index) != Invalid;
        }

        // A change of more than 5 degrees happened during this minute
        bool PostureChange(uint16_t index) const {
          return GetBit(changeBits, Slot(index));
        }

      private:
        uint16_t Slot(uint16_t index) const {
          return (newest + capacity + 1 - count + index) % capacity;
        }

        const uint8_t* medians;
        const uint8_t* changeBits;
        uint16_t capacity;
        uint16_t newest;
        uint16_t count;
      };

      struct Scratch {
        uint8_t* windowBits; // Size() / 8 + 1 bytes
        uint8_t* asleepBits; // Size() / 8 + 1 bytes
        uint16_t* histogram; // 256 entries
      };

      struct Session {
        bool found = false;
        bool ongoing = false;
        uint16_t start = 0; // Window index of the first minute of the sleep period
        uint16_t end = 0;   // Window index of the last minute
        uint16_t asleep = 0;
        uint16_t awake = 0;
        uint8_t wakeUps = 0;
        uint8_t threshold = 0; // HDCZA threshold that was used, DeltaUnits
      };

      Session Analyze(const Window& window, const Scratch& scratch);

      // Only valid after Analyze() with the same scratch
      Stage StageAt(const Window& window, const Scratch& scratch, const Session& session, uint16_t index);
    }
  }
}
