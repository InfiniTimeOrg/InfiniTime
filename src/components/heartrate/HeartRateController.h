#pragma once

#include <cstdint>
#include <components/ble/HeartRateService.h>
#include "utility/CircularBuffer.h"
#include "utility/Math.h"
#include "components/fs/FS.h"
#include <array>

namespace Pinetime {
  namespace Applications {
    class HeartRateTask;
  }

  namespace System {
    class SystemTask;
  }

  namespace Controllers {
    template <typename T>
    struct HeartRateZones {
      // 1440 minutes in a day (11 bits), 86400 seconds (17 bits)
      std::array<T, 5> zoneTime = {};

      T totalTime() const {
        return zoneTime[0] + zoneTime[1] + zoneTime[2] + zoneTime[3] + zoneTime[4];
      }
    };

    constexpr uint8_t maxHeartRateEstimate(uint8_t age) {
      return 220 - age;
    };

    constexpr int16_t fixed_rounding(int16_t value, int16_t divisor) {
      // true evil: we use >>'s signed behavior to propagate the leading 1 across all bits, eg: we have 0xffff or 0x0000
      int16_t signed_value = value >> 15u;
      int16_t half_divisor = (divisor / 2);
      return (value + half_divisor - (divisor & signed_value)) / divisor; // we replace the * by "1" with an &
    }

    template <size_t N>
    constexpr std::array<uint8_t, N> bpmZones(std::array<uint8_t, N>& percentages, uint8_t maxBeatsPerMinute) {
      std::array<uint8_t, N> targets {};
      const uint16_t bpm = maxBeatsPerMinute;
      for (uint32_t i = 0; i < N; i++) {
        targets[i++] = Utility::RoundedDiv((uint16_t)(percentages[i] * bpm), (uint16_t)100);
      }
      return targets;
    };

    struct HeartRateZoneSettings {
      uint32_t version = 0;
      uint32_t adjustMsDelay = 300000;     // 5 minutes
      uint32_t exerciseMsTarget = 1800000; // hour 3600'000
      uint8_t age = 25;
      uint8_t maxHeartRate = 195;
      std::array<uint8_t, 5> percentTarget = {50, 60, 70, 80, 90};
      std::array<uint8_t, 5> bpmTarget = {98, 117, 137, 156, 176};
      bool allowCalibration = true;

      HeartRateZoneSettings();
    };

    class HeartRateController {
    public:
      enum class States : uint8_t { Stopped, NotEnoughData, NoTouch, Running };

      HeartRateController(Pinetime::Controllers::FS& fs);
      void Enable();
      void Disable();
      void Update(States newState, uint8_t heartRate);

      void SetHeartRateTask(Applications::HeartRateTask* task);

      States State() const {
        return state;
      }

      uint8_t HeartRate() const {
        return heartRate;
      }

      uint8_t Zone() const {
        return zone;
      }

      HeartRateZones<uint32_t> Activity() const {
        return currentActivity;
      }

      HeartRateZoneSettings hrzSettings() const {
        return zSettings;
      }

      void SetService(Pinetime::Controllers::HeartRateService* service);

      void AdvanceDay();
      void SaveSettingsToFile() const;
      void LoadSettingsFromFile();

    private:
      Applications::HeartRateTask* task = nullptr;
      States state = States::Stopped;
      uint8_t heartRate = 0;
      uint8_t restingHeartRate = 0;
      Pinetime::Controllers::HeartRateService* service = nullptr;
      Pinetime::Controllers::FS& fs;

      HeartRateZoneSettings zSettings = {};
      uint32_t lastActiveTime = 0;
      // Heart Rate Zone Storage
      HeartRateZones<uint32_t> currentActivity = {};
      Utility::CircularBuffer<HeartRateZones<uint16_t>, 31> activity = {};
      uint8_t zone = 0;
    };
  }
}