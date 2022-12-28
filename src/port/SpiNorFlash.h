#pragma once
#include "drivers/SpiNorFlash.h"

#ifdef TARGET_DEVICE_PINETIME
  #include <drivers/spiFlash/SpiNorFlash.h>
#endif

namespace Pinetime {
  namespace Drivers {
#ifdef TARGET_DEVICE_PINETIME
    using SpiNorFlash = Interface::SpiNorFlash<Pinetime::Drivers::SpiFlash::SpiNorFlash>;
#else
  #error "No target device specified!"
#endif
  }
}
