#pragma once

namespace Pinetime {
  namespace System {
      enum class Messages {
        GoToSleep,
        GoToRunning,
        OnNewTime,
        OnNewNotification,
        OnTimerDone,
        OnNewCall,
        BleConnected,
        UpdateTimeOut,
        BleFirmwareUpdateStarted,
        BleFirmwareUpdateFinished,
        OnTouchEvent,
        OnButtonEvent,
        OnDisplayTaskSleeping,
        EnableSleeping,
        DisableSleeping,
        OnNewDay,
        OnChargingEvent
      };
    }
}
