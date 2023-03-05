#pragma once

#include <cstdint>

namespace Pinetime {
  namespace Applications {
    constexpr uint64_t rightShift(uint64_t nb) {
      return UINT64_C(1) << nb;
    }

    enum class Apps : uint64_t {
      None = rightShift(63),
      Launcher = rightShift(62),
      Clock = rightShift(61),
      SysInfo = rightShift(60),
      FirmwareUpdate = rightShift(59),
      FirmwareValidation = rightShift(58),
      NotificationsPreview = rightShift(57),
      Notifications = rightShift(56),
      Timer = rightShift(55),
      Alarm = rightShift(54),
      FlashLight = rightShift(53),
      BatteryInfo = rightShift(52),
      HeartRate = rightShift(51),
      StopWatch = rightShift(50),
      Steps = rightShift(49),
      PassKey = rightShift(48),
      QuickSettings = rightShift(47),
      Settings = rightShift(46),
      SettingWatchFace = rightShift(45),
      SettingTimeFormat = rightShift(44),
      SettingDisplay = rightShift(43),
      SettingWakeUp = rightShift(42),
      SettingSteps = rightShift(41),
      SettingSetDateTime = rightShift(40),
      SettingChimes = rightShift(39),
      SettingShakeThreshold = rightShift(38),
      SettingBluetooth = rightShift(37),
      Error = rightShift(36),

      Music = rightShift(0),
      Paint = rightShift(1),
      Paddle = rightShift(2),
      Twos = rightShift(3),
      Navigation = rightShift(4),
      Metronome = rightShift(5),
      Motion = rightShift(6),
    };

#ifndef DISABLED_APPS
    constexpr uint64_t disabledApps = static_cast<uint64_t>(Apps::Motion);
#else
    constexpr uint64_t disabledApps = DISABLED_APPS;
#endif

    constexpr bool isDisabled(Apps app) {
      return disabledApps & static_cast<uint64_t>(app);
    }
  }
}
