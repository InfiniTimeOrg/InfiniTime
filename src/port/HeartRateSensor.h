#pragma once
#include "drivers/HeartRateSensor.h"

#ifdef TARGET_DEVICE_PINETIME
  #include <drivers/heartRateSensors/Hrs3300.h>
#endif

namespace Pinetime {
  namespace Drivers {
#ifdef TARGET_DEVICE_PINETIME
    using HeartRateSensor = Interface::HeartRateSensor<Pinetime::Drivers::HeartRateSensors::Hrs3300>;
#else
  #error "No target device specified!"
#endif
  }
}
