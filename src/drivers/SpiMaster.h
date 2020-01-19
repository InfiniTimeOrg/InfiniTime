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

        SpiMaster(const SpiModule spi, const Parameters& params);
        bool Init();
        bool Write(const uint8_t* data, size_t size);
        bool WriteFast(const uint8_t* data, size_t size);
        void setup_workaround_for_ftpan_58(NRF_SPIM_Type *spim, uint32_t ppi_channel, uint32_t gpiote_channel);

        void Sleep();
        void Wakeup();

        bool GetStatusEnd();
        bool GetStatusStarted();

      private:
        NRF_SPIM_Type *  spiBaseAddress;
        uint8_t pinCsn;

        SpiMaster::SpiModule spi;
        SpiMaster::Parameters params;
    };
  }
}
