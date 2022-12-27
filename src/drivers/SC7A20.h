#pragma once

#include "drivers/AccelerationSensor.h"

namespace Pinetime {
  namespace Drivers {
    class SC7A20 : public AccelerationSensor {
    public:
      SC7A20(TwiMaster& twiMaster, uint8_t twiAddress);
      SC7A20(const SC7A20&) = delete;
      SC7A20& operator=(const SC7A20&) = delete;
      SC7A20(SC7A20&&) = delete;
      SC7A20& operator=(SC7A20&&) = delete;

      void Init();
      AccelerationValues Process();
    };
  }
}
