#pragma once
#include "drivers/Spi.h"

#ifdef TARGET_DEVICE_PINETIME
  #include <drivers/nrf52/Spi.h>
#endif

namespace Pinetime {
  namespace Drivers {
#ifdef TARGET_DEVICE_PINETIME
    using Spi = Interface::Spi<Pinetime::Drivers::Nrf52::Spi>;
#else
  #error "No target device specified!"
#endif
  }
}
