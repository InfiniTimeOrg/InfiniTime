#pragma once
#include <FreeRTOS.h>
#include <semphr.h>
#include <drivers/include/nrfx_twi.h> // NRF_TWIM_Type
#include <cstdint>
#include <nrfx_twim.h>

namespace Pinetime {
  namespace Drivers {
    class TwiMaster {
      public:
        enum class Modules { TWIM1 };
        enum class ErrorCodes {NoError, TransactionFailed};
        struct Parameters {
          uint32_t frequency;
          uint8_t pinSda;
          uint8_t pinScl;
        };

        TwiMaster(const Modules module, const Parameters& params);

        void Init();
        ErrorCodes Read(uint8_t deviceAddress, uint8_t registerAddress, uint8_t* buffer, size_t size);
        ErrorCodes Write(uint8_t deviceAddress, uint8_t registerAddress, const uint8_t* data, size_t size);

        void Sleep();
        void Wakeup();

      private:
        nrfx_twim_t twim;
        const Modules module;
        const Parameters params;
        SemaphoreHandle_t mutex;
        static constexpr uint8_t maxDataSize{8};
        static constexpr uint8_t registerSize{1};
        uint8_t internalBuffer[maxDataSize + registerSize];
    };
  }
}