#pragma once
#include <cstdint>
#include <cstddef>
#include <functional>
#include "drivers/SpiMaster.h"

namespace Pinetime {
  namespace Drivers {
    class Spi {
    public:
      Spi(SpiMaster& spiMaster, uint8_t pinCsn);
      Spi(const Spi&) = delete;
      Spi& operator=(const Spi&) = delete;
      Spi(Spi&&) = delete;
      Spi& operator=(Spi&&) = delete;

      bool Init();
      bool Write(const uint8_t* data, size_t size, const std::function<void()>& preTransactionHook);
      bool Read(uint8_t* cmd, size_t cmdSize, uint8_t* data, size_t dataSize);
      bool WriteCmdAndBuffer(const uint8_t* cmd, size_t cmdSize, const uint8_t* data, size_t dataSize);
      void Sleep();
      void Wakeup();

    private:
      SpiMaster& spiMaster;
      uint8_t pinCsn;
    };
  }
}
