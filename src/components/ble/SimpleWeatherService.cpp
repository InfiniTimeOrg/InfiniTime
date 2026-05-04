/*  Copyright (C) 2023 Jean-François Milants

    This file is part of InfiniTime.

    InfiniTime is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    InfiniTime is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "components/ble/SimpleWeatherService.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <nrf_log.h>

using namespace Pinetime::Controllers;

namespace {
  enum class MessageType : uint8_t { CurrentWeather, Forecast, Unknown };

  uint64_t ToUInt64(const uint8_t* data) {
    return data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24) + (static_cast<uint64_t>(data[4]) << 32) +
           (static_cast<uint64_t>(data[5]) << 40) + (static_cast<uint64_t>(data[6]) << 48) + (static_cast<uint64_t>(data[7]) << 56);
  }

  int16_t ToInt16(const uint8_t* data) {
    return data[0] + (data[1] << 8);
  }

  SimpleWeatherService::CurrentWeather CreateCurrentWeather(const uint8_t* dataBuffer) {
    SimpleWeatherService::Location cityName;
    std::memcpy(cityName.data(), &dataBuffer[16], 32);
    cityName[32] = '\0';
    int16_t sunrise = -1;
    int16_t sunset = -1;
    if (dataBuffer[1] > 0) {
      int16_t bufferSunrise = ToInt16(&dataBuffer[49]);
      int16_t bufferSunset = ToInt16(&dataBuffer[51]);

      // Sunrise/sunset format

      // Assume sun is down at minute 0 / midnight

      // 0<=x<1440 sunrise happens this many minutes into the day
      // (0 day starts with sun up)
      // -1 unknown
      // -2 sun not rising today

      // 0<x<1440 sunset happens this many minutes into the day
      // -1 unknown
      // -2 sun not setting today

      // Check if the weather data is well formed
      // Disable boolean simplification suggestion, as simplifying it makes it unreadable
      if (!( // NOLINT(readability-simplify-boolean-expr)
             // Fail if either unknown
            (bufferSunrise == -1 || bufferSunset == -1)
            // Cannot be out of range
            || (bufferSunrise < -2 || bufferSunrise > 1439 || bufferSunset < -2 || bufferSunset > 1439)
            // Cannot have sunset without sunrise
            || (bufferSunrise == -2 && bufferSunset != -2)
            // Cannot have sunset before sunrise
            || (bufferSunrise >= bufferSunset && bufferSunrise >= 0 && bufferSunset >= 0))) {
        sunrise = bufferSunrise;
        sunset = bufferSunset;
      }
    }
    return SimpleWeatherService::CurrentWeather(ToUInt64(&dataBuffer[2]),
                                                SimpleWeatherService::Temperature(ToInt16(&dataBuffer[10])),
                                                SimpleWeatherService::Temperature(ToInt16(&dataBuffer[12])),
                                                SimpleWeatherService::Temperature(ToInt16(&dataBuffer[14])),
                                                SimpleWeatherService::Icons {dataBuffer[16 + 32]},
                                                std::move(cityName),
                                                sunrise,
                                                sunset);
  }

  SimpleWeatherService::Forecast CreateForecast(const uint8_t* dataBuffer) {
    auto timestamp = static_cast<uint64_t>(ToUInt64(&dataBuffer[2]));

    std::array<std::optional<SimpleWeatherService::Forecast::Day>, SimpleWeatherService::MaxNbForecastDays> days;
    const uint8_t nbDaysInBuffer = dataBuffer[10];
    const uint8_t nbDays = std::min(SimpleWeatherService::MaxNbForecastDays, nbDaysInBuffer);
    for (int i = 0; i < nbDays; i++) {
      days[i] = SimpleWeatherService::Forecast::Day {SimpleWeatherService::Temperature(ToInt16(&dataBuffer[11 + (i * 5)])),
                                                     SimpleWeatherService::Temperature(ToInt16(&dataBuffer[13 + (i * 5)])),
                                                     SimpleWeatherService::Icons {dataBuffer[15 + (i * 5)]}};
    }
    return SimpleWeatherService::Forecast {timestamp, nbDays, days};
  }

  MessageType GetMessageType(const uint8_t* data) {
    auto messageType = static_cast<MessageType>(*data);
    if (messageType > MessageType::Unknown) {
      return MessageType::Unknown;
    }
    return messageType;
  }

  uint8_t GetVersion(const uint8_t* dataBuffer) {
    return dataBuffer[1];
  }
}

int WeatherCallback(uint16_t /*connHandle*/, uint16_t /*attrHandle*/, struct ble_gatt_access_ctxt* ctxt, void* arg) {
  return static_cast<Pinetime::Controllers::SimpleWeatherService*>(arg)->OnCommand(ctxt);
}

SimpleWeatherService::SimpleWeatherService(DateTime& dateTimeController) : dateTimeController(dateTimeController) {
}

void SimpleWeatherService::Init() {
  ble_gatts_count_cfg(serviceDefinition);
  ble_gatts_add_svcs(serviceDefinition);
}

int SimpleWeatherService::OnCommand(struct ble_gatt_access_ctxt* ctxt) {
  const auto* buffer = ctxt->om;
  const auto* dataBuffer = buffer->om_data;

  switch (GetMessageType(dataBuffer)) {
    case MessageType::CurrentWeather:
      if (GetVersion(dataBuffer) <= 1) {
        currentWeather = CreateCurrentWeather(dataBuffer);
        NRF_LOG_INFO("Current weather :\n\tTimestamp : %d\n\tTemperature:%d\n\tMin:%d\n\tMax:%d\n\tIcon:%d\n\tLocation:%s",
                     currentWeather->timestamp,
                     currentWeather->temperature.PreciseCelsius(),
                     currentWeather->minTemperature.PreciseCelsius(),
                     currentWeather->maxTemperature.PreciseCelsius(),
                     currentWeather->iconId,
                     currentWeather->location.data());
        if (GetVersion(dataBuffer) == 1) {
          NRF_LOG_INFO("Sunrise: %d\n\tSunset: %d", currentWeather->sunrise, currentWeather->sunset);
        }
      }
      break;
    case MessageType::Forecast:
      if (GetVersion(dataBuffer) == 0) {
        forecast = CreateForecast(dataBuffer);
        NRF_LOG_INFO("Forecast : Timestamp : %d", forecast->timestamp);
        for (int i = 0; i < 5; i++) {
          NRF_LOG_INFO("\t[%d] Min: %d - Max : %d - Icon : %d",
                       i,
                       forecast->days[i]->minTemperature.PreciseCelsius(),
                       forecast->days[i]->maxTemperature.PreciseCelsius(),
                       forecast->days[i]->iconId);
        }
      }
      break;
    default:
      break;
  }

  return 0;
}

std::optional<SimpleWeatherService::CurrentWeather> SimpleWeatherService::Current() const {
  if (currentWeather) {
    auto currentTime = dateTimeController.CurrentDateTime().time_since_epoch();
    auto weatherTpSecond = std::chrono::seconds {currentWeather->timestamp};
    auto weatherTp = std::chrono::duration_cast<std::chrono::seconds>(weatherTpSecond);
    auto delta = currentTime - weatherTp;

    if (delta < std::chrono::hours {24}) {
      return currentWeather;
    }
  }
  return {};
}

std::optional<SimpleWeatherService::Forecast> SimpleWeatherService::GetForecast() const {
  if (forecast) {
    auto currentTime = dateTimeController.CurrentDateTime().time_since_epoch();
    auto weatherTpSecond = std::chrono::seconds {forecast->timestamp};
    auto weatherTp = std::chrono::duration_cast<std::chrono::seconds>(weatherTpSecond);
    auto delta = currentTime - weatherTp;

    if (delta < std::chrono::hours {24}) {
      return this->forecast;
    }
  }
  return {};
}

bool SimpleWeatherService::IsNight() const {
  if (currentWeather && currentWeather->sunrise != -1 && currentWeather->sunset != -1) {
    auto currentTime = dateTimeController.CurrentDateTime().time_since_epoch();

    // Get timestamp for last midnight
    auto midnight = std::chrono::floor<std::chrono::days>(currentTime);

    // Calculate minutes since midnight
    auto currentMinutes = std::chrono::duration_cast<std::chrono::minutes>(currentTime - midnight).count();

    // Sun not rising today => night all hours
    if (currentWeather->sunrise == -2) {
      return true;
    }
    // Sun not setting today => check before sunrise
    if (currentWeather->sunset == -2) {
      return currentMinutes < currentWeather->sunrise;
    }

    // Before sunrise or after sunset
    return currentMinutes < currentWeather->sunrise || currentMinutes >= currentWeather->sunset;
  }

  return false;
}

bool SimpleWeatherService::CurrentWeather::operator==(const SimpleWeatherService::CurrentWeather& other) const {
  return this->iconId == other.iconId && this->temperature == other.temperature && this->timestamp == other.timestamp &&
         this->maxTemperature == other.maxTemperature && this->minTemperature == other.maxTemperature &&
         std::strcmp(this->location.data(), other.location.data()) == 0 && this->sunrise == other.sunrise && this->sunset == other.sunset;
}

bool SimpleWeatherService::Forecast::Day::operator==(const SimpleWeatherService::Forecast::Day& other) const {
  return this->iconId == other.iconId && this->maxTemperature == other.maxTemperature && this->minTemperature == other.maxTemperature;
}

bool SimpleWeatherService::Forecast::operator==(const SimpleWeatherService::Forecast& other) const {
  for (int i = 0; i < this->nbDays; i++) {
    if (this->days[i] != other.days[i]) {
      return false;
    }
  }
  return this->timestamp == other.timestamp && this->nbDays == other.nbDays;
}
