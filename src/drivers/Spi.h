#pragma once
#include <FreeRTOS.h>
#include <cstdint>
#include <cstddef>
#include <array>
#include <atomic>
#include <task.h>

#include "BufferProvider.h"
#include "SpiMaster.h"

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
        bool Write(const uint8_t* data, size_t size);
        bool Read(uint8_t* data, size_t size);
        void Sleep();
        void Wakeup();

      private:
        SpiMaster& spiMaster;
        uint8_t pinCsn;
    };
  }
}
