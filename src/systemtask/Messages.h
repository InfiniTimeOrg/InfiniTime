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
        OnButtonPushed,
        OnButtonLongPressed,
        OnButtonLongerPressed,
        OnButtonDoubleClicked,
        OnDisplayTaskSleeping,
        EnableSleeping,
        DisableSleeping,
        OnNewDay,
        OnChargingEvent,
        ReloadIdleTimer,
        SetOffAlarm,
        StopRinging,
        MeasureBatteryTimerExpired,
        BatteryMeasurementDone,
      };
    }
}
