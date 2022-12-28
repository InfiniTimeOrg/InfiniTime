#pragma once
#include "drivers/SpiMaster.h"

#ifdef TARGET_DEVICE_PINETIME
  #include <drivers/nrf52/SpiMaster.h>
#endif

namespace Pinetime {
  namespace Drivers {
#ifdef TARGET_DEVICE_PINETIME
    using SpiMaster = Interface::SpiMaster<Pinetime::Drivers::Nrf52::SpiMaster>;
#else
  #error "No target device specified!"
#endif
  }
}
