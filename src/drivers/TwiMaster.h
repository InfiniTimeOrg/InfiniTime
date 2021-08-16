#pragma once
#include <FreeRTOS.h>
#include <semphr.h>
#include <drivers/include/nrfx_twi.h> // NRF_TWIM_Type
#include <cstdint>

namespace Pinetime {
  namespace Drivers {
    class TwiMaster {
    public:
      enum class ErrorCodes { NoError, TransactionFailed };

      TwiMaster(NRF_TWIM_Type* module, uint32_t frequency, uint8_t pinSda, uint8_t pinScl);

      void Init();
      ErrorCodes Read(uint8_t deviceAddress, uint8_t registerAddress, uint8_t* buffer, size_t size);
      ErrorCodes Write(uint8_t deviceAddress, uint8_t registerAddress, const uint8_t* data, size_t size);

      void Sleep();
      void Wakeup();

    private:
      ErrorCodes Read(uint8_t deviceAddress, uint8_t* buffer, size_t size, bool stop);
      ErrorCodes Write(uint8_t deviceAddress, const uint8_t* data, size_t size, bool stop);
      void FixHwFreezed();
      void ConfigurePins() const;

      NRF_TWIM_Type* twiBaseAddress;
      SemaphoreHandle_t mutex = nullptr;
      NRF_TWIM_Type* module;
      uint32_t frequency;
      uint8_t pinSda;
      uint8_t pinScl;
      static constexpr uint8_t maxDataSize {16};
      static constexpr uint8_t registerSize {1};
      uint8_t internalBuffer[maxDataSize + registerSize];
      uint32_t txStartedCycleCount = 0;
      static constexpr uint32_t HwFreezedDelay {161000};
    };
  }
}
