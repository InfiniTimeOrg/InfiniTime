#pragma once
#include "displayapp/AvailableApps.h"

namespace Pinetime {
  namespace Applications {
    enum class Apps {
      None,
      Launcher,
      Clock,
      SysInfo,
      FirmwareUpdate,
      FirmwareValidation,
      NotificationsPreview,
      Notifications,
      Timer,
      Alarm,
      FlashLight,
      BatteryInfo,
#if APP_MUSIC
      Music,
#endif
#if APP_PAINT
      Paint,
#endif
#if APP_PADDLE
      Paddle,
#endif
#if APP_TWOS
      Twos,
#endif
      HeartRate,
#if APP_NAVIGATION
      Navigation,
#endif
      StopWatch,
#if APP_METRONOME
      Metronome,
#endif
#if APP_MOTION
      Motion,
#endif
      Steps,
#if APP_WEATHER
      Weather,
#endif
      PassKey,
      QuickSettings,
      Settings,
      SettingWatchFace,
      SettingTimeFormat,
      SettingDisplay,
      SettingWakeUp,
      SettingSteps,
      SettingSetDate,
      SettingSetTime,
      SettingChimes,
      SettingShakeThreshold,
      SettingBluetooth,
      Error
    };
  }
}
