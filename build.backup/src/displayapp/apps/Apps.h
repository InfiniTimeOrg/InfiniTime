#pragma once
#include <cstddef>
#include <cstdint>

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
      Twos,
      HeartRate,
      Sleep,
      Navigation,
      StopWatch,
      Metronome,
      Motion,
      Steps,
      Dice,
      Weather,
      PassKey,
      QuickSettings,
      Settings,
      SettingWatchFace,
      SettingTimeFormat,
      SettingWeatherFormat,
      SettingHeartRate,
      SettingDisplay,
      SettingWakeUp,
      SettingSteps,
      SettingSetDateTime,
      SettingChimes,
      SettingShakeThreshold,
      SettingBluetooth,
      Error
    };

    enum class WatchFace : uint8_t {
      Digital,
      Analog,
      PineTimeStyle,
      Terminal,
      Infineat,
      CasioStyleG7710,
      PrimeTime
    };

    template <Apps>
    struct AppTraits {};

    template <WatchFace>
    struct WatchFaceTraits {};

    template <Apps... As>
    struct TypeList {
      static constexpr size_t Count = sizeof...(As);
    };

    using UserAppTypes = TypeList<Apps::StopWatch, Apps::Alarm, Apps::Timer, Apps::Steps, Apps::HeartRate, Apps::Music, Apps::Paint, Apps::Paddle, Apps::Twos, Apps::Dice, Apps::Metronome, Apps::Navigation, Apps::Weather>;

    template <WatchFace... Ws>
    struct WatchFaceTypeList {
      static constexpr size_t Count = sizeof...(Ws);
    };

    using UserWatchFaceTypes = WatchFaceTypeList<WatchFace::Digital, WatchFace::Analog, WatchFace::PineTimeStyle, WatchFace::Terminal, WatchFace::Infineat, WatchFace::CasioStyleG7710, WatchFace::PrimeTime>;

    static_assert(UserWatchFaceTypes::Count >= 1);
  }
}
