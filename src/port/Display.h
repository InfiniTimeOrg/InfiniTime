#pragma once
#include "drivers/Display.h"

#ifdef TARGET_DEVICE_PINETIME
  #include <drivers/displays/St7789.h>
#endif

namespace Pinetime {
  namespace Drivers {
#ifdef TARGET_DEVICE_PINETIME
    using Display = Interface::Display<Pinetime::Drivers::Displays::St7789>;
#else
  #error "No target device specified!"
#endif
  }
}
