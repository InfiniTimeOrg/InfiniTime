#pragma once

#include <cstdint>

namespace Pinetime {
  namespace Applications {
    enum class Apps : uint64_t {
      None = 1ull << 63ull,
      Launcher = 1ull << 62ull,
      Clock = 1ull << 61ull,
      SysInfo = 1ull << 60ull,
      FirmwareUpdate = 1ull << 59ull,
      FirmwareValidation = 1ull << 58ull,
      NotificationsPreview = 1ull << 57ull,
      Notifications = 1ull << 56ull,
      Timer = 1ull << 55ull,
      Alarm = 1ull << 54ull,
      FlashLight = 1ull << 53ull,
      BatteryInfo = 1ull << 52ull,
      HeartRate = 1ull << 51ull,
      StopWatch = 1ull << 50ull,
      Steps = 1ull << 49ull,
      PassKey = 1ull << 48ull,
      QuickSettings = 1ull << 47ull,
      Settings = 1ull << 46ull,
      SettingWatchFace = 1ull << 45ull,
      SettingTimeFormat = 1ull << 44ull,
      SettingDisplay = 1ull << 43ull,
      SettingWakeUp = 1ull << 42ull,
      SettingSteps = 1ull << 41ull,
      SettingSetDateTime = 1ull << 40ull,
      SettingChimes = 1ull << 39ull,
      SettingShakeThreshold = 1ull << 38ull,
      SettingBluetooth = 1ull << 37ull,
      Error = 1ull << 36ull,

      Music = 1ull << 0ull,
      Paint = 1ull << 1ull,
      Paddle = 1ull << 2ull,
      Twos = 1ull << 3ull,
      Navigation = 1ull << 4ull,
      Metronome = 1ull << 5ull,
      Motion = 1ull << 6ull,
    };

#ifndef DISABLED_APPS
    constexpr uint64_t disabledApps = static_cast<uint64_t>(Apps::Motion);
#else
    constexpr uint64_t disabledApps = DISABLED_APPS;
#endif
  }
}
