#include "displayapp/screens/WeatherSymbols.h"
#include "displayapp/localization/Localization.h"

using namespace Pinetime::Applications::Localization;

const char* Pinetime::Applications::Screens::Symbols::GetSymbol(const Pinetime::Controllers::SimpleWeatherService::Icons icon,
                                                                const bool isNight) {
  switch (icon) {
    case Pinetime::Controllers::SimpleWeatherService::Icons::Sun:
      if (isNight) {
        return Symbols::moon;
      }
      return Symbols::sun;
      break;
    case Pinetime::Controllers::SimpleWeatherService::Icons::CloudsSun:
      if (isNight) {
        return Symbols::cloudMoon;
      }
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
      if (isNight) {
        return Symbols::cloudMoonRain;
      }
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

const char* Pinetime::Applications::Screens::Symbols::GetCondition(const Pinetime::Controllers::SimpleWeatherService::Icons icon,
                                                                   Language language) {
  switch (icon) {
    case Pinetime::Controllers::SimpleWeatherService::Icons::Sun:
      return Translate(language, StringId::ClearSky);
    case Pinetime::Controllers::SimpleWeatherService::Icons::CloudsSun:
      return Translate(language, StringId::FewClouds);
    case Pinetime::Controllers::SimpleWeatherService::Icons::Clouds:
      return Translate(language, StringId::ScatteredClouds);
    case Pinetime::Controllers::SimpleWeatherService::Icons::BrokenClouds:
      return Translate(language, StringId::BrokenClouds);
    case Pinetime::Controllers::SimpleWeatherService::Icons::CloudShowerHeavy:
      return Translate(language, StringId::ShowerRain);
    case Pinetime::Controllers::SimpleWeatherService::Icons::CloudSunRain:
      return Translate(language, StringId::Rain);
    case Pinetime::Controllers::SimpleWeatherService::Icons::Thunderstorm:
      return Translate(language, StringId::Thunderstorm);
    case Pinetime::Controllers::SimpleWeatherService::Icons::Snow:
      return Translate(language, StringId::Snow);
    case Pinetime::Controllers::SimpleWeatherService::Icons::Smog:
      return Translate(language, StringId::Mist);
    default:
      return "";
  }
}

const char* Pinetime::Applications::Screens::Symbols::GetSimpleCondition(const Pinetime::Controllers::SimpleWeatherService::Icons icon,
                                                                         Language language) {
  switch (icon) {
    case Pinetime::Controllers::SimpleWeatherService::Icons::Sun:
    case Pinetime::Controllers::SimpleWeatherService::Icons::CloudsSun:
      return Translate(language, StringId::Clear);
    case Pinetime::Controllers::SimpleWeatherService::Icons::Clouds:
    case Pinetime::Controllers::SimpleWeatherService::Icons::BrokenClouds:
      return Translate(language, StringId::Cloudy);
    case Pinetime::Controllers::SimpleWeatherService::Icons::CloudShowerHeavy:
      return Translate(language, StringId::Showers);
    case Pinetime::Controllers::SimpleWeatherService::Icons::CloudSunRain:
      return Translate(language, StringId::Rain);
    case Pinetime::Controllers::SimpleWeatherService::Icons::Thunderstorm:
      return Translate(language, StringId::Thunder);
    case Pinetime::Controllers::SimpleWeatherService::Icons::Snow:
      return Translate(language, StringId::Snow);
    case Pinetime::Controllers::SimpleWeatherService::Icons::Smog:
      return Translate(language, StringId::Mist);
    default:
      return "";
  }
}
