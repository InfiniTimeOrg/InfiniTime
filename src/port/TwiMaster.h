#pragma once
#include "drivers/TwiMaster.h"

#ifdef TARGET_DEVICE_PINETIME
  #include <drivers/nrf52/TwiMaster.h>
#endif

namespace Pinetime {
  namespace Drivers {
#ifdef TARGET_DEVICE_PINETIME
    using TwiMaster = Interface::TwiMaster<Pinetime::Drivers::Nrf52::TwiMaster>;
#else
  #error "No target device specified!"
#endif
  }
}
