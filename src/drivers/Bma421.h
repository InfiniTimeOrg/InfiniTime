#pragma once

#include "drivers/AccelerationSensor.h"
#include <drivers/Bma421_C/bma4_defs.h>

namespace Pinetime {
  namespace Drivers {
    class Bma421 : public AccelerationSensor {
    public:
      Bma421(TwiMaster& twiMaster, uint8_t twiAddress);
      Bma421(const Bma421&) = delete;
      Bma421& operator=(const Bma421&) = delete;
      Bma421(Bma421&&) = delete;
      Bma421& operator=(Bma421&&) = delete;

      /// The chip freezes the TWI bus after the softreset operation. Softreset is separated from the
      /// Init() method to allow the caller to uninit and then reinit the TWI device after the softreset.
      void SoftReset();
      void Init();
      AccelerationValues Process();
      void ResetStepCounter();

    private:
      bool isResetOk = false;
      struct bma4_dev bma;
    };
  }
}
