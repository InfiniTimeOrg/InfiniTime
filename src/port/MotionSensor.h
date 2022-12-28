#pragma once
#include "drivers/MotionSensor.h"

#ifdef TARGET_DEVICE_PINETIME
  #include <drivers/Bma421.h>
#endif

namespace Pinetime {
  namespace Drivers {
#ifdef TARGET_DEVICE_PINETIME
    using MotionSensor = Interface::MotionSensor<Pinetime::Drivers::MotionSensors::Bma421>;
#else
  #error "No target device specified!"
#endif
  }
}
