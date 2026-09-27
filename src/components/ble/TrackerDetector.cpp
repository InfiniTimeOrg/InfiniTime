#include "components/ble/TrackerDetector.h"

#include <cstring>

using namespace Pinetime::Controllers;

namespace {
  constexpr uint8_t adTypeUuid16Incomplete = 0x02;
  constexpr uint8_t adTypeUuid16Complete = 0x03;
  constexpr uint8_t adTypeServiceData16 = 0x16;
  constexpr uint8_t adTypeManufacturer = 0xFF;

  constexpr uint16_t companyApple = 0x004C;
  constexpr uint8_t appleOfflineFinding = 0x12;
  constexpr uint8_t appleSeparatedLength = 0x19; // Full key: separated from the owner. Near the owner it's 0x02.

  constexpr uint16_t uuidSmartTag = 0xFD5A;
  constexpr uint16_t uuidTile = 0xFEED;
  constexpr uint16_t uuidTileAlt = 0xFEEC;
  constexpr uint16_t uuidChipolo = 0xFE33;
  constexpr uint16_t uuidEddystone = 0xFEAA; // Google Find My Device network uses frame types 0x40/0x41
  constexpr uint16_t uuidDult = 0xFCB2;

  uint16_t Le16(const uint8_t* p) {
    return static_cast<uint16_t>(p[0] | (p[1] << 8));
  }

  TrackerDetector::Type FromUuid(uint16_t uuid, const uint8_t* serviceData, uint8_t serviceDataLength) {
    switch (uuid) {
      case uuidSmartTag:
        return TrackerDetector::Type::SmartTag;
      case uuidTile:
      case uuidTileAlt:
        return TrackerDetector::Type::Tile;
      case uuidChipolo:
        return TrackerDetector::Type::Chipolo;
      case uuidDult:
        return TrackerDetector::Type::Dult;
      case uuidEddystone:
        // Plain Eddystone beacons (frame types 0x00-0x30) are not trackers
        if (serviceData != nullptr && serviceDataLength >= 1 && (serviceData[0] == 0x40 || serviceData[0] == 0x41)) {
          return TrackerDetector::Type::GoogleFind;
        }
        return TrackerDetector::Type::None;
      default:
        return TrackerDetector::Type::None;
    }
  }
}

TrackerDetector::Type TrackerDetector::Classify(const uint8_t* data, uint8_t length) {
  uint8_t offset = 0;
  while (offset + 1 < length) {
    const uint8_t fieldLength = data[offset];
    if (fieldLength == 0 || offset + 1 + fieldLength > length) {
      break;
    }
    const uint8_t type = data[offset + 1];
    const uint8_t* payload = &data[offset + 2];
    const uint8_t payloadLength = fieldLength - 1;

    if (type == adTypeManufacturer && payloadLength >= 5 && Le16(payload) == companyApple && payload[2] == appleOfflineFinding &&
        payload[3] == appleSeparatedLength) {
      // Bits 4-5 of the status byte tell what kind of Find My device this is
      switch ((payload[4] >> 4) & 0x3) {
        case 1:
          return Type::AirTag;
        case 2:
          return Type::FindMy;
        case 3:
          return Type::AirPods;
        default:
          break; // iPhones, Macs... not trackers
      }
    } else if (type == adTypeServiceData16 && payloadLength >= 2) {
      const Type found = FromUuid(Le16(payload), payload + 2, payloadLength - 2);
      if (found != Type::None) {
        return found;
      }
    } else if (type == adTypeUuid16Incomplete || type == adTypeUuid16Complete) {
      for (uint8_t i = 0; i + 1 < payloadLength; i += 2) {
        const uint16_t uuid = Le16(&payload[i]);
        // Service data is needed to recognize Google tags, a bare UUID is not enough
        if (uuid != uuidEddystone) {
          const Type found = FromUuid(uuid, nullptr, 0);
          if (found != Type::None) {
            return found;
          }
        }
      }
    }
    offset += fieldLength + 1;
  }
  return Type::None;
}

const char* TrackerDetector::Name(Type type) {
  switch (type) {
    case Type::AirTag:
      return "AirTag";
    case Type::FindMy:
      return "Find My tag";
    case Type::AirPods:
      return "AirPods";
    case Type::SmartTag:
      return "SmartTag";
    case Type::Tile:
      return "Tile";
    case Type::Chipolo:
      return "Chipolo";
    case Type::GoogleFind:
      return "Google tag";
    case Type::Dult:
      return "Tracker";
    case Type::None:
      break;
  }
  return "?";
}

void TrackerDetector::OnSighting(const uint8_t* address, Type type, int8_t rssi, uint32_t now) {
  if (type == Type::None) {
    return;
  }
  Tracker* tracker = nullptr;
  for (size_t i = 0; i < count; i++) {
    if (std::memcmp(trackers[i].address, address, 6) == 0) {
      tracker = &trackers[i];
      break;
    }
  }
  if (tracker == nullptr) {
    if (count < MaxTrackers) {
      tracker = &trackers[count++];
    } else {
      // Full: replace the one not seen for the longest time
      tracker = &trackers[0];
      for (size_t i = 1; i < count; i++) {
        if (trackers[i].lastSeen < tracker->lastSeen) {
          tracker = &trackers[i];
        }
      }
    }
    std::memcpy(tracker->address, address, 6);
    tracker->type = type;
    tracker->firstSeen = now;
    tracker->scans = 0;
    tracker->lastScan = static_cast<uint16_t>(scanId - 1);
    tracker->alerted = false;
  }
  tracker->rssi = rssi;
  tracker->lastSeen = now;
  if (tracker->lastScan != scanId) {
    tracker->lastScan = scanId;
    if (tracker->scans < 255) {
      tracker->scans++;
    }
  }
}

bool TrackerDetector::Evaluate(uint32_t now, size_t& alertIndex) {
  // Forget trackers that went away, keeping the array packed
  size_t kept = 0;
  for (size_t i = 0; i < count; i++) {
    if (now - trackers[i].lastSeen <= ForgetSeconds) {
      trackers[kept++] = trackers[i];
    }
  }
  count = kept;

  for (size_t i = 0; i < count; i++) {
    if (!trackers[i].alerted && trackers[i].IsFollowing()) {
      trackers[i].alerted = true;
      alertIndex = i;
      return true;
    }
  }
  return false;
}
