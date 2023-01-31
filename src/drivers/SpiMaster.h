#pragma once
#include <cstddef>
#include <cstdint>

#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>

namespace Pinetime {
  namespace Drivers {
    class SpiMaster {
    public:
      enum class SpiModule : uint8_t { SPI0, SPI1 };
      enum class BitOrder : uint8_t { Msb_Lsb, Lsb_Msb };
      enum class Modes : uint8_t { Mode0, Mode1, Mode2, Mode3 };
      enum class Frequencies : uint8_t { Freq8Mhz };

      struct Parameters {
        BitOrder bitOrder;
        Modes mode;
        Frequencies Frequency;
        uint8_t pinSCK;
        uint8_t pinMOSI;
        uint8_t pinMISO;
      };

      SpiMaster(const SpiModule spi, const Parameters& params);
      SpiMaster(const SpiMaster&) = delete;
      SpiMaster& operator=(const SpiMaster&) = delete;
      SpiMaster(SpiMaster&&) = delete;
      SpiMaster& operator=(SpiMaster&&) = delete;

      bool Init();
      bool Write(uint8_t pinCsn, const uint8_t* data, size_t size);
      bool Read(uint8_t pinCsn, uint8_t* cmd, size_t cmdSize, uint8_t* data, size_t dataSize);

      bool WriteCmdAndBuffer(uint8_t pinCsn, const uint8_t* cmd, size_t cmdSize, const uint8_t* data, size_t dataSize);

      void OnStartedEvent();
      void OnEndEvent();

      void Sleep();
      void Wakeup();

    private:
      void SetupWorkaroundForFtpan58(NRF_SPIM_Type* spim, uint32_t ppi_channel, uint32_t gpiote_channel);
      void DisableWorkaroundForFtpan58(NRF_SPIM_Type* spim, uint32_t ppi_channel, uint32_t gpiote_channel);
      void PrepareTx(const volatile uint32_t bufferAddress, const volatile size_t size);
      void PrepareRx(const volatile uint32_t bufferAddress, const volatile size_t size);

      NRF_SPIM_Type* spiBaseAddress;
      uint8_t pinCsn;

      SpiMaster::SpiModule spi;
      SpiMaster::Parameters params;

      volatile uint32_t currentBufferAddr = 0;
      volatile size_t currentBufferSize = 0;
      volatile TaskHandle_t taskToNotify;
      SemaphoreHandle_t mutex = nullptr;
    };
  }
}
