#pragma once

#include "drivers/TwiMaster.h"

namespace Pinetime {
  namespace Drivers {
    class Cst816S {
    public:
      enum class Gestures : uint8_t {
        None = 0x00,
        SlideDown = 0x01,
        SlideUp = 0x02,
        SlideLeft = 0x03,
        SlideRight = 0x04,
        SingleTap = 0x05,
        DoubleTap = 0x0B,
        LongPress = 0x0C
      };

      struct TouchInfos {
        uint16_t x = 0;
        uint16_t y = 0;
        Gestures gesture = Gestures::None;
        bool touching = false;
        bool isValid = false;
      };

      Cst816S(TwiMaster& twiMaster, uint8_t twiAddress);
      Cst816S(const Cst816S&) = delete;
      Cst816S& operator=(const Cst816S&) = delete;
      Cst816S(Cst816S&&) = delete;
      Cst816S& operator=(Cst816S&&) = delete;

      bool Init();
      TouchInfos GetTouchInfo();
      void Sleep();
      void Wakeup();

      uint8_t GetChipId() const {
        return chipId;
      }

      uint8_t GetVendorId() const {
        return vendorId;
      }

      uint8_t GetFwVersion() const {
        return fwVersion;
      }

    private:
      bool CheckDeviceIds();

      // Unused/Unavailable commented out
      static constexpr uint8_t gestureIndex = 1;
      static constexpr uint8_t touchPointNumIndex = 2;
      // static constexpr uint8_t touchEventIndex = 3;
      static constexpr uint8_t touchXHighIndex = 3;
      static constexpr uint8_t touchXLowIndex = 4;
      // static constexpr uint8_t touchIdIndex = 5;
      static constexpr uint8_t touchYHighIndex = 5;
      static constexpr uint8_t touchYLowIndex = 6;
      // static constexpr uint8_t touchStep = 6;
      // static constexpr uint8_t touchXYIndex = 7;
      // static constexpr uint8_t touchMiscIndex = 8;

      static constexpr uint8_t maxX = 240;
      static constexpr uint8_t maxY = 240;

      TwiMaster& twiMaster;
      uint8_t twiAddress;

      uint8_t chipId;
      uint8_t vendorId;
      uint8_t fwVersion;
    };

  }
}
