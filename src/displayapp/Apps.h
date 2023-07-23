#pragma once

namespace Pinetime {
  namespace Applications {
    enum class Apps : uint8_t {
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
      Music,
      Paint,
      Paddle,
      HeartRate,
      Navigation,
      StopWatch,
      Metronome,
      Motion,
      Steps,
      PassKey,
      QuickSettings,
      Settings,
      SettingWatchFace,
      SettingTimeFormat,
      SettingDisplay,
      SettingWakeUp,
      SettingSteps,
      SettingSetDateTime,
      SettingChimes,
      SettingShakeThreshold,
      SettingBluetooth,
      Error,
      Dynamic,
    };
  }
}
