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
#include <algorithm>
#include "SimpleWeatherService.h"
#include <cstring>
#include <nrf_log.h>
#include <array>

using namespace Pinetime::Controllers;

namespace {
  enum class MessageType { CurrentWeather, Forecast, Unknown };

  SimpleWeatherService::CurrentWeather CreateCurrentWeather(const uint8_t* dataBuffer) {
    char cityName[33];
    std::memcpy(&cityName[0], &dataBuffer[13], 32);
    cityName[32] = '\0';
    return SimpleWeatherService::CurrentWeather {dataBuffer[2] + (dataBuffer[3] << 8) + (dataBuffer[4] << 16) + (dataBuffer[5] << 24) +
                                                   ((uint64_t) dataBuffer[6] << 32) + ((uint64_t) dataBuffer[7] << 40) +
                                                   ((uint64_t) dataBuffer[8] << 48) + ((uint64_t) dataBuffer[9] << 54),
                                                 dataBuffer[10],
                                                 dataBuffer[11],
                                                 dataBuffer[12],
                                                 dataBuffer[13 + 32],
                                                 cityName};
  }

  SimpleWeatherService::Forecast CreateForecast(const uint8_t* dataBuffer) {
    uint64_t timestamp = static_cast<uint64_t>(dataBuffer[2] + (dataBuffer[3] << 8) + (dataBuffer[4] << 16) + (dataBuffer[5] << 24) +
                                               ((uint64_t) dataBuffer[6] << 32) + ((uint64_t) dataBuffer[7] << 40) +
                                               ((uint64_t) dataBuffer[8] << 48) + ((uint64_t) dataBuffer[9] << 54));

    std::array<SimpleWeatherService::Forecast::Day, SimpleWeatherService::MaxNbForecastDays> days;
    const uint8_t nbDaysInBuffer = dataBuffer[10];
    const uint8_t nbDays = std::min(SimpleWeatherService::MaxNbForecastDays, nbDaysInBuffer);
    for (int i = 0; i < nbDays; i++) {
      days[i] = SimpleWeatherService::Forecast::Day {dataBuffer[11 + (i * 3)], dataBuffer[12 + (i * 3)], dataBuffer[13 + (i * 3)]};
    }
    return SimpleWeatherService::Forecast {timestamp, nbDays, days};
  }

  MessageType GetMessageType(const uint8_t* dataBuffer) {
    switch (dataBuffer[0]) {
      case 0:
        return MessageType::CurrentWeather;
        break;
      case 1:
        return MessageType::Forecast;
        break;
      default:
        return MessageType::Unknown;
        break;
    }
  }

  uint8_t GetVersion(const uint8_t* dataBuffer) {
    return dataBuffer[1];
  }
}

int WeatherCallback(uint16_t /*connHandle*/, uint16_t /*attrHandle*/, struct ble_gatt_access_ctxt* ctxt, void* arg) {
  return static_cast<Pinetime::Controllers::SimpleWeatherService*>(arg)->OnCommand(ctxt);
}

SimpleWeatherService::SimpleWeatherService(const DateTime& dateTimeController) : dateTimeController(dateTimeController) {
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
      if (GetVersion(dataBuffer) == 0) {
        currentWeather = CreateCurrentWeather(dataBuffer);
        NRF_LOG_INFO("Current weather :\n\tTimestamp : %d\n\tTemperature:%d\n\tMin:%d\n\tMax:%d\n\tIcon:%d\n\tLocation:%s",
                     currentWeather->timestamp,
                     currentWeather->temperature,
                     currentWeather->minTemperature,
                     currentWeather->maxTemperature,
                     currentWeather->iconId,
                     currentWeather->location);
      }
      break;
    case MessageType::Forecast:
      if (GetVersion(dataBuffer) == 0) {
        forecast = CreateForecast(dataBuffer);
        NRF_LOG_INFO("Forecast : Timestamp : %d", forecast->timestamp);
        for (int i = 0; i < 5; i++) {
          NRF_LOG_INFO("\t[%d] Min: %d - Max : %d - Icon : %d",
                       i,
                       forecast->days[i].minTemperature,
                       forecast->days[i].maxTemperature,
                       forecast->days[i].iconId);
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
    auto currentTime = dateTimeController.UTCDateTime().time_since_epoch();
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
    auto currentTime = dateTimeController.UTCDateTime().time_since_epoch();
    auto weatherTpSecond = std::chrono::seconds {forecast->timestamp};
    auto weatherTp = std::chrono::duration_cast<std::chrono::seconds>(weatherTpSecond);
    auto delta = currentTime - weatherTp;

    if (delta < std::chrono::hours {24}) {
      return this->forecast;
    }
  }
  return {};
}

bool SimpleWeatherService::CurrentWeather::operator==(const SimpleWeatherService::CurrentWeather& other) const {
  return this->iconId == other.iconId && this->temperature == other.temperature && this->timestamp == other.timestamp &&
         this->maxTemperature == other.maxTemperature && this->minTemperature == other.maxTemperature;
}

bool SimpleWeatherService::CurrentWeather::operator!=(const SimpleWeatherService::CurrentWeather& other) const {
  return !operator==(other);
}
