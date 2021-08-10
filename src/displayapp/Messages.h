#pragma once
namespace Pinetime {
  namespace Applications {
    namespace Display {
      enum class Messages : uint8_t {
        GoToSleep,
        GoToRunning,
        UpdateDateTime,
        UpdateBleConnection,
        UpdateBatteryLevel,
        TouchEvent,
        ButtonPushed,
        NewNotification,
        TimerDone,
        BleFirmwareUpdateStarted,
        UpdateTimeOut,
        DimScreen,
        RestoreBrightness
      };
    }
  }
}
