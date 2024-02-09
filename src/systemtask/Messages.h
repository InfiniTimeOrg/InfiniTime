#pragma once
#include <cstdint>

namespace Pinetime {
  namespace System {
    enum class Messages : uint8_t {
      GoToSleep,
      GoToRunning,
      TouchWakeUp,
      OnNewTime,
      OnNewNotification,
      OnNewCall,
      BleConnected,
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
      OnMusicStarted,
      OnNavChange,
      SetOffAlarm,
      MeasureBatteryTimerExpired,
      BatteryPercentageUpdated,
      StartFileTransfer,
      StopFileTransfer,
      BleRadioEnableToggle
    };
  }
}
