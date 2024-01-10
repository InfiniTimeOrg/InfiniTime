#pragma once
#include "components/ble/SimpleWeatherService.h"
#include "displayapp/screens/Symbols.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      namespace Symbols {
        const char* GetSymbol(const Pinetime::Controllers::SimpleWeatherService::Icons icon) {
          switch (icon) {
            case Pinetime::Controllers::SimpleWeatherService::Icons::Sun:
              return Symbols::sun;
              break;
            case Pinetime::Controllers::SimpleWeatherService::Icons::CloudsSun:
              return Symbols::cloudSun;
              break;
            case Pinetime::Controllers::SimpleWeatherService::Icons::Clouds:
              return Symbols::cloud;
              break;
            case Pinetime::Controllers::SimpleWeatherService::Icons::BrokenClouds:
              return Symbols::cloudMeatball;
              break;
            case Pinetime::Controllers::SimpleWeatherService::Icons::Thunderstorm:
              return Symbols::bolt;
              break;
            case Pinetime::Controllers::SimpleWeatherService::Icons::Snow:
              return Symbols::snowflake;
              break;
            case Pinetime::Controllers::SimpleWeatherService::Icons::CloudShowerHeavy:
              return Symbols::cloudShowersHeavy;
              break;
            case Pinetime::Controllers::SimpleWeatherService::Icons::CloudSunRain:
              return Symbols::cloudSunRain;
              break;
            case Pinetime::Controllers::SimpleWeatherService::Icons::Smog:
              return Symbols::smog;
              break;
            default:
              return Symbols::ban;
              break;
          }
        }
      }
    }
  }
}
