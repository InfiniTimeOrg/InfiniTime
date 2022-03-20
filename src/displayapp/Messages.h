#pragma once
#include <cstdint>
namespace Pinetime {
  namespace Applications {
    namespace Display {
      enum class Messages : uint8_t {
        GoToSleep,
        GoToRunning,
        TouchEvent,
        ButtonPushed,
        TimerDone,
        DimScreen,
        RestoreBrightness,
        AlarmTriggered,
      };
    }
  }
}
