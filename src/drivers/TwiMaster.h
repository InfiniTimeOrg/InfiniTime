#pragma once
#include <FreeRTOS.h>
#include <semphr.h>
#include <drivers/include/nrfx_twi.h>


namespace Pinetime {
  namespace Drivers {
    class TwiMaster {
      public:
        enum class Modules { TWIM1 };
        enum class Frequencies {Khz100, Khz250, Khz400};
        struct Parameters {
          uint32_t frequency;
          uint8_t pinSda;
          uint8_t pinScl;
        };

        TwiMaster(const Modules module, const Parameters& params);

        void Init();
        void Read(uint8_t deviceAddress, uint8_t registerAddress, uint8_t* buffer, size_t size);
        void Write(uint8_t deviceAddress, uint8_t registerAddress, const uint8_t* data, size_t size);

      private:
        void Read(uint8_t deviceAddress, uint8_t* buffer, size_t size, bool stop);
        void Write(uint8_t deviceAddress, const uint8_t* data, size_t size, bool stop);
        NRF_TWIM_Type* twiBaseAddress;
        SemaphoreHandle_t mutex;
        const Modules module;
        const Parameters params;
        static constexpr uint8_t maxDataSize{8};
        static constexpr uint8_t registerSize{1};
        uint8_t internalBuffer[maxDataSize + registerSize];

    };
  }
}