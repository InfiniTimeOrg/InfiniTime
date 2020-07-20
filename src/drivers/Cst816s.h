#pragma once

#include <nrfx_twi.h>
#include "TwiMaster.h"

namespace Pinetime {
  namespace Drivers {
    class Cst816S {
      public :
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
          uint16_t x;
          uint16_t y;
          uint8_t action;
          uint8_t finger;
          uint8_t pressure;
          uint8_t area;
          Gestures gesture;
          bool isTouch = false;
        };

        Cst816S(TwiMaster& twiMaster, uint8_t twiAddress);
        Cst816S(const Cst816S&) = delete;
        Cst816S& operator=(const Cst816S&) = delete;
        Cst816S(Cst816S&&) = delete;
        Cst816S& operator=(Cst816S&&) = delete;

        void Init();
        TouchInfos GetTouchInfo();
        void Sleep();
        void Wakeup();
      private:
        static constexpr uint8_t pinIrq = 28;
        static constexpr uint8_t pinReset = 10;
        static constexpr uint8_t lastTouchId = 0x0f;
        static constexpr uint8_t touchPointNumIndex = 2;
        static constexpr uint8_t touchMiscIndex = 8;
        static constexpr uint8_t touchXYIndex = 7;
        static constexpr uint8_t touchEventIndex = 3;
        static constexpr uint8_t touchXHighIndex = 3;
        static constexpr uint8_t touchXLowIndex = 4;
        static constexpr uint8_t touchYHighIndex = 5;
        static constexpr uint8_t touchYLowIndex = 6;
        static constexpr uint8_t touchIdIndex = 5;
        static constexpr uint8_t touchStep = 6;
        static constexpr uint8_t gestureIndex = 1;

        uint8_t touchData[63];
        TwiMaster& twiMaster;
        uint8_t twiAddress;
    };

  }
}