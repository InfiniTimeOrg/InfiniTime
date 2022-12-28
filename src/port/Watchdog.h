#pragma once
#include "drivers/Watchdog.h"

#ifdef TARGET_DEVICE_PINETIME
  #include <drivers/nrf52//Watchdog.h>
#endif

namespace Pinetime {
  namespace Drivers {
#ifdef TARGET_DEVICE_PINETIME
    using Watchdog = Interface::Watchdog<Pinetime::Drivers::Nrf52::Watchdog>;
#else
  #error "No target device specified!"
#endif
  }
}
