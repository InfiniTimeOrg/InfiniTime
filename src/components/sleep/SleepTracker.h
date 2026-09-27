#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <FreeRTOS.h>
#include <semphr.h>
#include "components/sleep/SleepAnalysis.h"

namespace Pinetime {
  namespace Controllers {
    class FS;

    // Records wrist movement per minute for the last 24 hours, detects sleep from it
    // and keeps a short history of nights on the filesystem.
    class SleepTracker {
    public:
      static constexpr uint16_t Minutes = 24 * 60;
      static constexpr uint8_t EpochMinutes = 5; // Resolution of the stored hypnogram
      static constexpr uint8_t MaxEpochs = 192;  // 16 hours
      static constexpr uint8_t MaxNights = 7;

      struct Night {
        uint32_t start;  // Local time, seconds since epoch
        uint16_t inBed;  // Minutes from falling asleep to waking up
        uint16_t asleep; // Minutes
        uint16_t still;
        uint16_t awake;
        uint8_t avgHeartRate; // 0 when unknown
        uint8_t minHeartRate;
        uint8_t epochs;
        uint8_t ongoing;
        std::array<uint8_t, MaxEpochs / 4> stages; // 2 bits per epoch, Sleep::Stage values

        Sleep::Stage EpochStage(uint8_t epoch) const {
          return static_cast<Sleep::Stage>((stages[epoch / 4] >> ((epoch % 4) * 2)) & 0x3);
        }
      };

      explicit SleepTracker(FS& fs);

      void Load();
      void Save();

      bool IsDirty() const {
        return dirty;
      }

      // Called by the system task for every accelerometer sample (10Hz)
      void OnMotion(int16_t x, int16_t y, int16_t z, uint32_t localMinute, bool offWrist, uint8_t heartRate);

      // Called by the system task every hour: stores the last night once it's over
      void Finalize();

      // Current or most recent session in the last 24 hours, false if none
      bool Live(Night& night);

      size_t HistoryCount() const {
        return historyCount;
      }

      // 0 is the most recent stored night
      Night History(size_t index) const;

    private:
      static constexpr uint16_t HeartRateSlots = Minutes / 10;
      static constexpr int16_t NoiseFloor = 24;             // Sample-to-sample change (mg, summed over axes) ignored as noise
      static constexpr uint8_t LevelDivider = 16;           // Movement sum per minute divided by this is the level
      static constexpr uint16_t OffWristStillMinutes = 150; // No movement at all for this long: watch is off the wrist
      static constexpr uint32_t magic = 0x50454c53;         // "SLEP"
      static constexpr uint8_t fileVersion = 1;
      static constexpr const char* fileName = "/sleep.dat";

      void CloseMinute();
      void AdvanceTo(uint32_t minute);
      bool Build(const Sleep::Session& session, Night& night);

      FS& fs;
      SemaphoreHandle_t mutex;

      std::array<uint8_t, Minutes> levels;
      std::array<uint8_t, Minutes / 8 + 1> asleepBits {};
      uint16_t newest = Minutes - 1;
      uint16_t count = 0;

      struct HeartRateSlot {
        uint16_t sum;
        uint8_t samples;
      };

      std::array<HeartRateSlot, HeartRateSlots> heartRates {};

      uint32_t currentMinute = 0;
      bool started = false;
      uint32_t movement = 0;
      bool minuteOffWrist = false;
      uint8_t minuteHeartRate = 0;
      uint16_t stillMinutes = 0;
      bool havePrevious = false;
      int16_t previousX = 0;
      int16_t previousY = 0;
      int16_t previousZ = 0;

      std::array<Night, MaxNights> history {};
      uint8_t historyCount = 0;
      bool dirty = false;
    };
  }
}
