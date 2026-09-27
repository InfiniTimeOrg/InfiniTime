#pragma once

#include <cstddef>
#include <cstdint>

// Motion based sleep scoring (actigraphy). Pure functions without FreeRTOS dependencies so they can be tested on a PC.
//
// Input is one activity level per minute: 0 means no movement above the sensor noise floor,
// higher values mean more movement. A minute is scored asleep when a weighted window of the
// surrounding minutes has little sustained movement (in the spirit of the Cole-Kripke algorithm):
// a turn-over is one or two active minutes, being awake is movement minute after minute.
// The longest block of sleep, allowing short wake-ups, is reported as the session.
// Stages are estimated from motion only: "still" means asleep without any movement for
// several minutes around it, "light" is asleep with small movements.
namespace Pinetime {
  namespace Controllers {
    namespace Sleep {
      static constexpr uint8_t LevelMax = 253;
      static constexpr uint8_t LevelOffWrist = 254;
      static constexpr uint8_t LevelNoData = 255;

      // Tuning
      static constexpr uint8_t ActiveLevel = 3;        // A minute above this level counts as active
      static constexpr uint8_t WindowBefore = 4;       // Scoring window: minutes before...
      static constexpr uint8_t WindowAfter = 2;        // ...and after the scored minute
      static constexpr uint8_t MaxActiveInWindow = 2;  // More active minutes than this in the window means awake
      static constexpr uint8_t OnsetMinutes = 10;      // Consecutive asleep minutes needed to start a session
      static constexpr uint8_t MaxWakeGapMinutes = 60; // A longer awake or off-wrist stretch ends the session
      static constexpr uint16_t MinAsleepMinutes = 60; // Shorter sessions are ignored
      static constexpr uint8_t OngoingMinutes = 15;    // Session ending this close to now is still in progress
      static constexpr uint8_t StillRadius = 5;        // Minutes on each side that must be motionless for "still"

      enum class Stage : uint8_t { None = 0, Awake = 1, Light = 2, Still = 3 };

      // Chronological view over a ring buffer: index 0 is the oldest minute, Size() - 1 the newest
      class Window {
      public:
        Window(const uint8_t* ring, uint16_t capacity, uint16_t newest, uint16_t count)
          : ring {ring}, capacity {capacity}, newest {newest}, count {count} {
        }

        uint16_t Size() const {
          return count;
        }

        uint8_t operator[](uint16_t index) const {
          return ring[(newest + capacity + 1 - count + index) % capacity];
        }

      private:
        const uint8_t* ring;
        uint16_t capacity;
        uint16_t newest;
        uint16_t count;
      };

      struct Session {
        bool found = false;
        bool ongoing = false;
        uint16_t start = 0; // Window index of the first asleep minute
        uint16_t end = 0;   // Window index of the last asleep minute
        uint16_t asleep = 0;
        uint16_t still = 0;
        uint16_t light = 0;
        uint16_t awake = 0;
      };

      // Scores every minute of the window into asleepBits (at least Size() / 8 + 1 bytes) and finds the main session
      Session Analyze(const Window& window, uint8_t* asleepBits);

      // Stage of a minute, only valid after Analyze() filled asleepBits
      Stage StageAt(const Window& window, const uint8_t* asleepBits, uint16_t index);

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
    }
  }
}
