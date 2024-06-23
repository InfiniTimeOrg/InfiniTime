#pragma once

#include "drivers/Cst816s_registers.h"
#include "drivers/TwiMaster.h"

namespace Pinetime {
  namespace Drivers {
    class Cst816S {
    public:
      enum class Gestures : uint8_t {
        None = GESTURE_ID_NONE,
        SlideDown = GESTURE_ID_SLIDE_DOWN,
        SlideUp = GESTURE_ID_SLIDE_UP,
        SlideLeft = GESTURE_ID_SLIDE_LEFT,
        SlideRight = GESTURE_ID_SLIDE_RIGHT,
        SingleTap = GESTURE_ID_SINGLE_TAP,
        DoubleTap = GESTURE_ID_DOUBLE_TAP,
        LongPress = GESTURE_ID_LONG_PRESS,
        Invalid = 0xFF
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
      static constexpr uint8_t maxX = 240;
      static constexpr uint8_t maxY = 240;

      TwiMaster& twiMaster;
      uint8_t twiAddress;

      uint8_t chipId;
      uint8_t vendorId;
      uint8_t fwVersion;

      bool firstEvent = true;
    };

  }
}
