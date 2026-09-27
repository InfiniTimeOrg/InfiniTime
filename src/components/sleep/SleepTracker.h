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

    // Records the forearm angle every 5 seconds, keeps per-minute angle statistics for the last
    // 24 hours, detects sleep from them (see SleepAnalysis.h) and keeps a short history of nights
    // on the filesystem.
    class SleepTracker {
    public:
      static constexpr uint16_t Minutes = 24 * 60;
      static constexpr uint8_t EpochMinutes = 5; // Resolution of the stored hypnogram
      static constexpr uint8_t MaxEpochs = 192;  // 16 hours
      static constexpr uint8_t MaxNights = 7;

      struct Night {
        uint32_t start;  // Local time, seconds since epoch
        uint16_t inBed;  // Minutes in the sleep period
        uint16_t asleep; // Minutes
        uint16_t awake;
        uint8_t wakeUps;
        uint8_t avgHeartRate; // 0 when unknown
        uint8_t minHeartRate;
        uint8_t epochs;
        uint8_t ongoing;
        uint8_t reserved;
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
      static constexpr uint8_t SamplesPerEpoch = 50; // 5 seconds at 10Hz
      static constexpr uint8_t EpochsPerMinute = 12;
      static constexpr int16_t NonWearRange = 50;    // mg: less than this on 2 of 3 axes...
      static constexpr uint16_t NonWearMinutes = 60; // ...for this long means the watch isn't worn (GGIR)
      static constexpr uint32_t magic = 0x50454c53;  // "SLEP"
      static constexpr uint8_t fileVersion = 2;
      static constexpr const char* fileName = "/sleep.dat";

      void CloseMinute();
      void AdvanceTo(uint32_t minute);
      void Push(uint8_t median, bool postureChange);
      void MarkInvalid(uint16_t minutes);
      Sleep::Window MakeWindow() const;
      Sleep::Scratch MakeScratch();
      void Build(const Sleep::Session& session, Night& night);

      FS& fs;
      SemaphoreHandle_t mutex;

      // Per minute, ring buffers
      std::array<uint8_t, Minutes> medians;
      std::array<uint8_t, Minutes / 8 + 1> changeBits {};
      uint16_t newest = Minutes - 1;
      uint16_t count = 0;

      // Analysis scratch space
      std::array<uint8_t, Minutes / 8 + 1> windowBits {};
      std::array<uint8_t, Minutes / 8 + 1> asleepBits {};
      std::array<uint16_t, 256> histogram {};

      struct HeartRateSlot {
        uint16_t sum;
        uint8_t samples;
      };

      std::array<HeartRateSlot, HeartRateSlots> heartRates {};

      uint32_t currentMinute = 0;
      bool started = false;

      // Current 5 second epoch
      int32_t sumX = 0;
      int32_t sumY = 0;
      int32_t sumZ = 0;
      uint8_t samples = 0;
      bool haveAngle = false;
      float previousAngle = 0;

      // Current minute
      std::array<uint8_t, EpochsPerMinute> deltas {};
      uint8_t deltaCount = 0;
      bool minutePostureChange = false;
      bool minuteOffWrist = false;
      uint8_t minuteHeartRate = 0;
      std::array<int16_t, 3> minuteMin {};
      std::array<int16_t, 3> minuteMax {};
      bool minuteHasSamples = false;

      // Non-wear detection: range of each axis since the start of the current still stretch
      std::array<int16_t, 3> stillMin {};
      std::array<int16_t, 3> stillMax {};
      uint16_t stillMinutes = 0;

      std::array<Night, MaxNights> history {};
      uint8_t historyCount = 0;
      bool dirty = false;
    };
  }
}
