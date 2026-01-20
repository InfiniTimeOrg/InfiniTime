#include "displayapp/screens/WeatherSymbols.h"

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

const char* Pinetime::Applications::Screens::Symbols::GetCondition(const Pinetime::Controllers::SimpleWeatherService::Icons icon) {
  switch (icon) {
    case Pinetime::Controllers::SimpleWeatherService::Icons::Sun:
      return "Clear sky";
    case Pinetime::Controllers::SimpleWeatherService::Icons::CloudsSun:
      return "Few clouds";
    case Pinetime::Controllers::SimpleWeatherService::Icons::Clouds:
      return "Scattered clouds";
    case Pinetime::Controllers::SimpleWeatherService::Icons::BrokenClouds:
      return "Broken clouds";
    case Pinetime::Controllers::SimpleWeatherService::Icons::CloudShowerHeavy:
      return "Shower rain";
    case Pinetime::Controllers::SimpleWeatherService::Icons::CloudSunRain:
      return "Rain";
    case Pinetime::Controllers::SimpleWeatherService::Icons::Thunderstorm:
      return "Thunderstorm";
    case Pinetime::Controllers::SimpleWeatherService::Icons::Snow:
      return "Snow";
    case Pinetime::Controllers::SimpleWeatherService::Icons::Smog:
      return "Mist";
    default:
      return "";
  }
}

const char* Pinetime::Applications::Screens::Symbols::GetSimpleCondition(const Pinetime::Controllers::SimpleWeatherService::Icons icon) {
  switch (icon) {
    case Pinetime::Controllers::SimpleWeatherService::Icons::Sun:
    case Pinetime::Controllers::SimpleWeatherService::Icons::CloudsSun:
      return "Clear";
    case Pinetime::Controllers::SimpleWeatherService::Icons::Clouds:
    case Pinetime::Controllers::SimpleWeatherService::Icons::BrokenClouds:
      return "Cloudy";
    case Pinetime::Controllers::SimpleWeatherService::Icons::CloudShowerHeavy:
      return "Showers";
    case Pinetime::Controllers::SimpleWeatherService::Icons::CloudSunRain:
      return "Rain";
    case Pinetime::Controllers::SimpleWeatherService::Icons::Thunderstorm:
      return "Thunder";
    case Pinetime::Controllers::SimpleWeatherService::Icons::Snow:
      return "Snow";
    case Pinetime::Controllers::SimpleWeatherService::Icons::Smog:
      return "Mist";
    default:
      return "";
  }
}
