#pragma once

namespace Pinetime {
  namespace System {
      enum class Messages {
        GoToSleep,
        GoToRunning,
        TouchWakeUp,
        OnNewTime,
        OnNewNotification,
        OnTimerDone,
        OnNewCall,
        BleConnected,
        UpdateTimeOut,
        BleFirmwareUpdateStarted,
        BleFirmwareUpdateFinished,
        OnTouchEvent,
        HandleButtonEvent,
        HandleButtonTimerEvent,
        OnDisplayTaskSleeping,
        EnableSleeping,
        DisableSleeping,
        OnNewDay,
        OnNewHour,
        OnNewHalfHour,
        OnChargingEvent,
        OnPairing,
        SetOffAlarm,
        StopRinging,
        MeasureBatteryTimerExpired,
        BatteryPercentageUpdated,
        StartFileTransfer,
        StopFileTransfer,
        AirplaneModeToggle
      };
    }
}
