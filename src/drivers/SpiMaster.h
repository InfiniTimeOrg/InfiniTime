#pragma once
#include <cstdint>
#include <cstddef>
#include <array>

namespace Pinetime {
  namespace Drivers {
    class SpiMaster {
      public:;
        enum class SpiModule : uint8_t {SPI0, SPI1};
        enum class BitOrder : uint8_t {Msb_Lsb, Lsb_Msb};
        enum class Modes : uint8_t {Mode0, Mode1, Mode2, Mode3};
        enum class Frequencies : uint8_t {Freq8Mhz};
        struct Parameters {
          BitOrder bitOrder;
          Modes mode;
          Frequencies Frequency;
          uint8_t pinSCK;
          uint8_t pinMOSI;
          uint8_t pinMISO;
          uint8_t pinCSN;
        };
        bool Init(const SpiModule spi, const Parameters& params);

        bool Write(const uint8_t* data, size_t size);

      private:
        NRF_SPI_Type *  spiBaseAddress;
        uint8_t pinCsn;
    };
  }
}
