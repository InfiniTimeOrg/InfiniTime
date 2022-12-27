#pragma once
#include <cstddef>
#include <cstdint>
#include "drivers/nrf52/SpiMaster.h"

namespace Pinetime {
  namespace Drivers {
    namespace Nrf52 {
      class Spi {
      public:
        Spi(Pinetime::Drivers::Nrf52::SpiMaster& spiMaster, uint8_t pinCsn);
        Spi(const Spi&) = delete;
        Spi& operator=(const Spi&) = delete;
        Spi(Spi&&) = delete;
        Spi& operator=(Spi&&) = delete;

        bool Init();
        bool Write(const uint8_t* data, size_t size);
        bool Read(uint8_t* cmd, size_t cmdSize, uint8_t* data, size_t dataSize);
        bool WriteCmdAndBuffer(const uint8_t* cmd, size_t cmdSize, const uint8_t* data, size_t dataSize);
        void Sleep();
        void Wakeup();

      private:
        Pinetime::Drivers::Nrf52::SpiMaster& spiMaster;
        uint8_t pinCsn;
      };
    }
  }
}
