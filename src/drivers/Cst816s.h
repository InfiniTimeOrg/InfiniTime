#pragma once

#include <nrfx_twi.h>

namespace Pinetime {
  namespace Drivers {
    class Cst816S {
      public :
        struct TouchInfos {
          uint16_t x;
          uint16_t y;
          uint8_t action;
          uint8_t finger;
          uint8_t pressure;
          uint8_t area;
          bool isTouch = false;
        };

        void Init();
        void Probe();
        TouchInfos GetTouchInfo();
      private:
        static constexpr uint8_t pinIrq = 28;
        static constexpr uint8_t pinReset = 10;
        static constexpr uint8_t address = 0x15;
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

        uint8_t touchData[63];

        // TODO TWI (i²C) should be created outside and injected into this class
        // It will be needed when implementing other I²C devices
        // (0x15 = touch, 0x18 = accelerometer, 0x44 = HR sensor)
        nrfx_twi_t twi = NRFX_TWI_INSTANCE(1); // Use instance 1, because instance 0 is already used by SPI
    };

  }
}
