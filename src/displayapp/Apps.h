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
#ifdef APP_MUSIC
      Music,
#endif
#ifdef APP_PAINT
      Paint,
#endif
#ifdef APP_PADDLE
      Paddle,
#endif
#ifdef APP_TWOS
      Twos,
#endif
      HeartRate,
#ifdef APP_NAVIGATION
      Navigation,
#endif
      StopWatch,
#ifdef APP_METRONOME
      Metronome,
#endif
#ifdef APP_MOTION
      Motion,
#endif
      Steps,
#ifdef APP_WEATHER
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
