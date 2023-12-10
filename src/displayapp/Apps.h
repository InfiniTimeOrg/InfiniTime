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
      Weather
    };

    enum class WatchFace : uint8_t {
      Digital = 0,
      Analog = 1,
      PineTimeStyle = 2,
      Terminal = 3,
      Infineat = 4,
      CasioStyleG7710 = 5,
    };

    template <Apps>
    struct AppTraits {};

    template <WatchFace>
    struct WatchFaceTraits {};

    template <Apps... As>
    struct TypeList {
      static constexpr size_t Count = sizeof...(As);
    };

    template <WatchFace... Ws>
    struct WatchFaceTypeList {
      static constexpr size_t Count = sizeof...(Ws);
    };

    using UserAppTypes = TypeList<Apps::StopWatch,
                                  Apps::Alarm,
                                  Apps::Timer,
                                  Apps::Steps,
                                  Apps::HeartRate,
                                  Apps::Music,
                                  Apps::Paint,
                                  Apps::Paddle,
                                  Apps::Twos,
                                  Apps::Metronome,
                                  Apps::Navigation
                                  /*
                                  Apps::Weather,
                                  Apps::Motion
                                  */
                                  >;

    using UserWatchFaceTypes = WatchFaceTypeList<WatchFace::Digital,
                                                 WatchFace::Analog,
                                                 WatchFace::PineTimeStyle,
                                                 WatchFace::Terminal,
                                                 WatchFace::Infineat,
                                                 WatchFace::CasioStyleG7710>;

    static_assert(UserWatchFaceTypes::Count >= 1);
  }
}
