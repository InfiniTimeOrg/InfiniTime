#pragma once
#include "drivers/TouchPanel.h"

#ifdef TARGET_DEVICE_PINETIME
  #include <drivers/touchpanels/Cst816s.h>
#endif

namespace Pinetime {
  namespace Drivers {
#ifdef TARGET_DEVICE_PINETIME
    using TouchPanel = Interface::Touchpanel<Pinetime::Drivers::TouchPanels::Cst816S>;
#else
  #error "No target device specified!"
#endif
  }
}
