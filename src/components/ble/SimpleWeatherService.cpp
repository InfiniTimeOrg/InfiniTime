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

//Linear gradient temperature color calculator :)

    int16_t SimpleWeatherService::RoundTemperature(int16_t temp) {
      return temp = temp / 100 + (temp % 100 >= 50 ? 1 : 0);
    }
  
    const char* floatToRgbHex(std::tuple<float, float, float> rgb) {
      char *rgbHex = new char[7];
      snprintf(rgbHex, 7, "%02X%02X%02X", static_cast<int>(std::get<0>(rgb)), static_cast<int>(std::get<1>(rgb)), static_cast<int>(std::get<2>(rgb)));
      return rgbHex;
    }
  
    std::tuple<float, float, float> hexToFloat(int rgb) {
      float r = ((rgb >> 16) & 0xFF);
      float g = ((rgb >> 8) & 0xFF);
      float b = (rgb & 0xFF);
      return std::tuple<float, float, float>(r, g, b);
    }
    
    float normalize(float value) {
    if (value < 0.0f) {
        return 0.0f;
    } else if (value > 1.0f) {
        return 1.0f;
    } else {
        return value;
    }
}
    
    // reference: https://dev.to/ndesmic/linear-color-gradients-from-scratch-1a0e
    std::tuple<float, float, float> lerp(std::tuple<float, float, float> pointA, std::tuple<float, float, float> pointB, float normalValue) {
      NRF_LOG_INFO("Normal value: %f", normalValue);
      auto lerpOutput = std::tuple<float, float, float>(
        get<0>(pointA) + (get<0>(pointB) - get<0>(pointA)) * normalValue,
        get<1>(pointA) + (get<1>(pointB) - get<1>(pointA)) * normalValue,
        get<2>(pointA) + (get<2>(pointB) - get<2>(pointA)) * normalValue
        //std::lerp(get<0>(pointA), get<0>(pointB), normalValue),
        //std::lerp(get<1>(pointA), get<1>(pointB), normalValue),
        //std::lerp(get<2>(pointA), get<2>(pointB), normalValue)
        );
      NRF_LOG_INFO("pointA: %f, %f, %f", get<0>(pointA), get<1>(pointA), get<2>(pointA));
      NRF_LOG_INFO("pointB: %f, %f, %f", get<0>(pointB), get<1>(pointB), get<2>(pointB));
      NRF_LOG_INFO("lerp: %f, %f, %f", get<0>(lerpOutput), get<1>(lerpOutput), get<2>(lerpOutput));
      return lerpOutput;
    }
    
    const char* SimpleWeatherService::TemperatureColor(int16_t temperature) {
      const std::vector<int> colors = {0x5555ff, 0x00c9ff, 0xff9b00, 0xff0000};
      std::vector<std::tuple<float, float, float>> stops;
      for (auto colorVal: colors) {
       stops.emplace_back(hexToFloat(colorVal));
      }
      int tempRounded = RoundTemperature(temperature);
      if (tempRounded < 0) {
         tempRounded = 1;
      }
      // convert temperature to range between newMin and newMax
      float oldMax = 50;
      float oldMin = 0;
      float newMax = 1;
      float newMin = 0;
      float oldRange = (oldMax - oldMin);
      float newRange = (newMax - newMin);
      float newValue = (((tempRounded - oldMin) * newRange) / oldRange) + newMin;
      newValue = normalize(newValue);
      if (newValue <= .33f) {
        return floatToRgbHex(lerp(stops[0], stops[1], newValue));
      } else if (newValue <= .66f) {
        return floatToRgbHex(lerp(stops[1], stops[2], newValue));
      } else {
        return floatToRgbHex(lerp(stops[2], stops[3], newValue));
      }
    }


bool SimpleWeatherService::CurrentWeather::operator==(const SimpleWeatherService::CurrentWeather& other) const {
  return this->iconId == other.iconId && this->temperature == other.temperature && this->timestamp == other.timestamp &&
         this->maxTemperature == other.maxTemperature && this->minTemperature == other.maxTemperature &&
         std::strcmp(this->location.data(), other.location.data()) == 0;
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
