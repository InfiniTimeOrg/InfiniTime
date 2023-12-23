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
    return SimpleWeatherService::CurrentWeather(ToUInt64(&dataBuffer[2]),
                                                ToInt16(&dataBuffer[10]),
                                                ToInt16(&dataBuffer[12]),
                                                ToInt16(&dataBuffer[14]),
                                                SimpleWeatherService::Icons {dataBuffer[16 + 32]},
                                                std::move(cityName));
  }

  SimpleWeatherService::Forecast CreateForecast(const uint8_t* dataBuffer) {
    auto timestamp = static_cast<uint64_t>(ToUInt64(&dataBuffer[2]));

    std::array<SimpleWeatherService::Forecast::Day, SimpleWeatherService::MaxNbForecastDays> days;
    const uint8_t nbDaysInBuffer = dataBuffer[10];
    const uint8_t nbDays = std::min(SimpleWeatherService::MaxNbForecastDays, nbDaysInBuffer);
    for (int i = 0; i < nbDays; i++) {
      days[i] = SimpleWeatherService::Forecast::Day {ToInt16(&dataBuffer[11 + (i * 5)]),
                                                     ToInt16(&dataBuffer[13 + (i * 5)]),
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
                     currentWeather->location.data());
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
         this->maxTemperature == other.maxTemperature && this->minTemperature == other.maxTemperature &&
         std::strcmp(this->location.data(), other.location.data()) == 0;
}
