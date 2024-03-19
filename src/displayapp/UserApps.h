#pragma once
#include "displayapp/apps/Apps.h"
#include "Controllers.h"

#include "displayapp/screens/Alarm.h"
#include "displayapp/screens/Dice.h"
#include "displayapp/screens/Timer.h"
#include "displayapp/screens/Twos.h"
#include "displayapp/screens/Tile.h"
#include "displayapp/screens/ApplicationList.h"
#include "displayapp/screens/WatchFaceDigital.h"
#include "displayapp/screens/WatchFaceAnalog.h"
#include "displayapp/screens/WatchFaceCasioStyleG7710.h"
#include "displayapp/screens/WatchFaceInfineat.h"
#include "displayapp/screens/WatchFacePineTimeStyle.h"
#include "displayapp/screens/WatchFaceTerminal.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class Screen;
    }

    struct AppDescription {
      Apps app;
      const char* icon;
      Screens::Screen* (*create)(AppControllers& controllers);
    };

    struct WatchFaceDescription {
      const char* name;
      Screens::Screen* (*create)(AppControllers& controllers);
      bool (*isAvailable)(Controllers::FS& fileSystem);
    };

    template <Apps t>
    consteval AppDescription CreateAppDescription() {
      return {AppTraits<t>::app, AppTraits<t>::icon, &AppTraits<t>::Create};
    }

    template <class t>
    consteval WatchFaceDescription CreateWatchFaceDescription() {
      static_assert(std::char_traits<char>::length(WatchFaceTraits<t>::name) < 16,
                    "The name of the watch faces is limited to 15 characters max");
      return {WatchFaceTraits<t>::name, &WatchFaceTraits<t>::Create, &WatchFaceTraits<t>::IsAvailable};
    }

    template <template <Apps...> typename T, Apps... ts>
    consteval std::array<AppDescription, sizeof...(ts)> CreateAppDescriptions(T<ts...>) {
      return {CreateAppDescription<ts>()...};
    }

    template <template <class...> typename T, class... ts>
    consteval std::array<WatchFaceDescription, sizeof...(ts)> CreateWatchFaceDescriptions(T<ts...>) {
      return {CreateWatchFaceDescription<ts>()...};
    }

    constexpr auto userApps = CreateAppDescriptions(UserAppTypes {});
    constexpr auto userWatchFaces = CreateWatchFaceDescriptions(UserWatchFaceTypes {});
  }
}
