#pragma once
#include <drivers/Bma421_C/bma4_defs.h>

namespace Pinetime {
  namespace Drivers {
    class TwiMaster;
    class Bma421 {
    public:
      struct Values {
        uint32_t steps;
        int16_t x;
        int16_t y;
        int16_t z;
      };
      Bma421(TwiMaster& twiMaster, uint8_t twiAddress);
      Bma421(const Bma421&) = delete;
      Bma421& operator=(const Bma421&) = delete;
      Bma421(Bma421&&) = delete;
      Bma421& operator=(Bma421&&) = delete;

      void Init();
      void Reset();
      Values Process();

      void Read(uint8_t registerAddress, uint8_t *buffer, size_t size);
      void Write(uint8_t registerAddress, const uint8_t *data, size_t size);

      bool IsOk() const;

    private:
      TwiMaster& twiMaster;
      uint8_t deviceAddress = 0x18;
      struct bma4_dev bma;
      bool isOk = false;
    };
  }
}