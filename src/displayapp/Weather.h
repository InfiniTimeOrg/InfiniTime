#pragma once

#include <cstdint>
#include <variant>

#include "components/ble/SimpleWeatherService.h"
#include "components/settings/Settings.h"

namespace Pinetime {
  namespace Applications {
    struct Temperature {
      int16_t temp;
    };

    Temperature Convert(Controllers::SimpleWeatherService::Temperature temp, Controllers::Settings::WeatherFormat format);
  }
}
