#pragma once
#include "components/ble/SimpleWeatherService.h"
#include "displayapp/localization/Localization.h"
#include "displayapp/screens/Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      namespace Symbols {
        const char* GetSymbol(const Pinetime::Controllers::SimpleWeatherService::Icons icon, const bool isNight);
        const char* GetCondition(const Pinetime::Controllers::SimpleWeatherService::Icons icon,
                                 Pinetime::Applications::Localization::Language language);
        const char* GetSimpleCondition(const Pinetime::Controllers::SimpleWeatherService::Icons icon,
                                       Pinetime::Applications::Localization::Language language);
      }
    }
  }
}
