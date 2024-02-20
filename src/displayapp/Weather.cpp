#include "displayapp/Weather.h"

using namespace Pinetime::Applications;

Temperature Pinetime::Applications::Convert(Controllers::SimpleWeatherService::Temperature temp,
                                            Controllers::Settings::WeatherFormat format) {
  Temperature t = {temp.temp};
  if (format == Controllers::Settings::WeatherFormat::Imperial) {
    t.temp = t.temp * 9 / 5 + 3200;
  }
  t.temp = t.temp / 100 + (t.temp % 100 >= 50 ? 1 : 0);
  return t;
}
