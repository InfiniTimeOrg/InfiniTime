#pragma once
#include <cstdint>

namespace Pinetime {
  namespace System {
    enum class Messages : uint8_t {
      GoToSleep,
      GoToRunning,
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
      OnDisplayTaskAOD,
      EnableSleeping,
      DisableSleeping,
      OnNewDay,
      OnNewHour,
      OnNewHalfHour,
      OnNewQuarterHour,
      OnChargingEvent,
      OnPairing,
      SetOffAlarm,
      MeasureBatteryTimerExpired,
      BatteryPercentageUpdated,
      StartFileTransfer,
      StopFileTransfer,
      BleRadioEnableToggle
    };
  }
}
