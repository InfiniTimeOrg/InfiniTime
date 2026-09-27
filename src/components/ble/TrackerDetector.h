#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace Pinetime {
  namespace Controllers {

    // Recognizes Bluetooth item trackers from their advertisements and notices when one keeps
    // showing up around the wearer. Plain C++ so it can be tested on a PC.
    //
    // Only trackers that advertise in a way that can indicate unwanted tracking are counted:
    // Apple Find My accessories only when separated from their owner (a tag near its owner is
    // just someone's keys), Samsung SmartTag, Tile, Chipolo, Google Find My Device tags and the
    // cross-vendor DULT unwanted tracking beacon.
    class TrackerDetector {
    public:
      enum class Type : uint8_t { None, AirTag, FindMy, AirPods, SmartTag, Tile, Chipolo, GoogleFind, Dult };

      static constexpr uint8_t MaxTrackers = 20;
      static constexpr uint32_t FollowSeconds = 30 * 60; // Seen this long...
      static constexpr uint8_t FollowScans = 3;          // ...in at least this many scans: it's following you
      static constexpr uint32_t ForgetSeconds = 20 * 60; // Not seen this long: forget it

      struct Tracker {
        uint8_t address[6];
        Type type;
        int8_t rssi;        // Last signal strength, dBm
        uint32_t firstSeen; // Seconds, monotonic
        uint32_t lastSeen;
        uint16_t lastScan;
        uint8_t scans; // Number of separate scans it showed up in
        bool alerted;

        uint32_t FollowingSeconds() const {
          return lastSeen - firstSeen;
        }

        bool IsFollowing() const {
          return FollowingSeconds() >= FollowSeconds && scans >= FollowScans;
        }
      };

      // Parses raw advertising data
      static Type Classify(const uint8_t* data, uint8_t length);
      static const char* Name(Type type);

      void BeginScan() {
        scanId++;
      }

      void OnSighting(const uint8_t* address, Type type, int8_t rssi, uint32_t now);

      // Drops trackers that went away. Returns true when a tracker newly qualifies as following,
      // alertIndex is then set to it.
      bool Evaluate(uint32_t now, size_t& alertIndex);

      size_t Count() const {
        return count;
      }

      Tracker Get(size_t index) const {
        return trackers[index];
      }

    private:
      std::array<Tracker, MaxTrackers> trackers {};
      size_t count = 0;
      uint16_t scanId = 0;
    };
  }
}
