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
        OnButtonEvent,
        OnDisplayTaskSleeping,
        EnableSleeping,
        DisableSleeping,
        OnNewDay,
        OnNewHour,
        OnNewHalfHour,
        OnChargingEvent,
        SetOffAlarm,
        StopRinging,
        MeasureBatteryTimerExpired,
        BatteryPercentageUpdated,
      };
    }
}
