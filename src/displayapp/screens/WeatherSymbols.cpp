#include "displayapp/screens/WeatherSymbols.h"

const char* Pinetime::Applications::Screens::Symbols::GetSymbol(const Pinetime::Controllers::SimpleWeatherService::Icons icon) {
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

const char* Pinetime::Applications::Screens::Symbols::GetSimpleCondition(const Pinetime::Controllers::SimpleWeatherService::Icons icon) {
  switch (icon) {
    case Pinetime::Controllers::SimpleWeatherService::Icons::Sun:
      return "Clear";
    case Pinetime::Controllers::SimpleWeatherService::Icons::CloudsSun:
      return "Cloudy";
    case Pinetime::Controllers::SimpleWeatherService::Icons::Clouds:
      return "Cloudy";
    case Pinetime::Controllers::SimpleWeatherService::Icons::BrokenClouds:
      return "Cloudy";
    case Pinetime::Controllers::SimpleWeatherService::Icons::CloudShowerHeavy:
      return "Rainy";
    case Pinetime::Controllers::SimpleWeatherService::Icons::CloudSunRain:
      return "Rainy";
    case Pinetime::Controllers::SimpleWeatherService::Icons::Thunderstorm:
      return "Stormy";
    case Pinetime::Controllers::SimpleWeatherService::Icons::Snow:
      return "Snowy";
    case Pinetime::Controllers::SimpleWeatherService::Icons::Smog:
     return "Misty";
    default:
      return "";
  }
}
