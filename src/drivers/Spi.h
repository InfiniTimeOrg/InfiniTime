#pragma once
#include <cstdint>
#include <cstddef>
#include "drivers/SpiMaster.h"

namespace Pinetime {
  namespace Drivers {
    class Spi {
    public:
      Spi(SpiMaster& spiMaster, uint8_t pinCsn);

      bool Write(const uint8_t* data, size_t size);
      bool Read(uint8_t* cmd, size_t cmdSize, uint8_t* data, size_t dataSize);
      bool WriteCmdAndBuffer(const uint8_t* cmd, size_t cmdSize, const uint8_t* data, size_t dataSize);

    private:
      SpiMaster& spiMaster;
      uint8_t pinCsn;
    };
  }
}
