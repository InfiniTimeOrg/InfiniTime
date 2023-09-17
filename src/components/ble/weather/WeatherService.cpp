/*  Copyright (C) 2021 Avamander

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
#include <qcbor/qcbor_spiffy_decode.h>
#include "WeatherService.h"
#include "libs/QCBOR/inc/qcbor/qcbor.h"

namespace {
  constexpr bool EnableObscuration = false;
  constexpr bool EnablePrecipitation = true;
  constexpr bool EnableWind = false;
  constexpr bool EnableTemperature = true;
  constexpr bool EnableAirQuality = false;
  constexpr bool EnableSpecial = false;
  constexpr bool EnablePressure = false;
  constexpr bool EnableLocation = false;
  constexpr bool EnableClouds = true;
  constexpr bool EnableHumidity = false;

  std::unique_ptr<Pinetime::Controllers::WeatherData::AirQuality>
  CreateAirQualityEvent(int64_t timestamp, uint32_t expires, const std::string& polluter, uint32_t amount) {
    auto airquality = std::make_unique<Pinetime::Controllers::WeatherData::AirQuality>();
    airquality->timestamp = timestamp;
    airquality->eventType = Pinetime::Controllers::WeatherData::eventtype::AirQuality;
    airquality->expires = expires;
    airquality->polluter = polluter;
    airquality->amount = amount; // NOLINT(bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)
    return airquality;
  }

  std::unique_ptr<Pinetime::Controllers::WeatherData::Obscuration>
  CreateObscurationEvent(int64_t timestamp, uint32_t expires, uint16_t amount) {
    auto obscuration = std::make_unique<Pinetime::Controllers::WeatherData::Obscuration>();
    obscuration->timestamp = timestamp;
    obscuration->eventType = Pinetime::Controllers::WeatherData::eventtype::Obscuration;
    obscuration->expires = expires;
    obscuration->amount = amount;
    return obscuration;
  }

  std::unique_ptr<Pinetime::Controllers::WeatherData::Precipitation>
  CreatePrecipitationEvent(int64_t timestamp,
                           uint32_t expires,
                           Pinetime::Controllers::WeatherData::precipitationtype type,
                           uint8_t amount) {
    auto precipitation = std::make_unique<Pinetime::Controllers::WeatherData::Precipitation>();
    precipitation->timestamp = timestamp;
    precipitation->eventType = Pinetime::Controllers::WeatherData::eventtype::Precipitation;
    precipitation->expires = expires;
    precipitation->type = type;
    precipitation->amount = amount;
    return precipitation;
  }

  std::unique_ptr<Pinetime::Controllers::WeatherData::Wind>
  CreateWindEvent(int64_t timestamp, uint32_t expires, uint8_t speedMin, uint8_t speedMax, uint8_t directionMin, uint8_t directionMax) {
    auto wind = std::make_unique<Pinetime::Controllers::WeatherData::Wind>();
    wind->timestamp = timestamp;
    wind->eventType = Pinetime::Controllers::WeatherData::eventtype::Wind;
    wind->expires = expires;
    wind->speedMin = speedMin;
    wind->speedMax = speedMax;
    wind->directionMin = directionMin;
    wind->directionMax = directionMax;
    return wind;
  }

  std::unique_ptr<Pinetime::Controllers::WeatherData::Temperature>
  CreateTemperatureEvent(int64_t timestamp, uint32_t expires, int16_t temperatureValue, int16_t dewPoint) {
    auto temperature = std::make_unique<Pinetime::Controllers::WeatherData::Temperature>();
    temperature->timestamp = timestamp;
    temperature->eventType = Pinetime::Controllers::WeatherData::eventtype::Temperature;
    temperature->expires = expires;
    temperature->temperature = temperatureValue;
    temperature->dewPoint = dewPoint;
    return temperature;
  }

  std::unique_ptr<Pinetime::Controllers::WeatherData::Special>
  CreateSpecialEvent(int64_t timestamp, uint32_t expires, Pinetime::Controllers::WeatherData::specialtype type) {
    auto special = std::make_unique<Pinetime::Controllers::WeatherData::Special>();
    special->timestamp = timestamp;
    special->eventType = Pinetime::Controllers::WeatherData::eventtype::Special;
    special->expires = expires;
    special->type = type;
    return special;
  }

  std::unique_ptr<Pinetime::Controllers::WeatherData::Pressure>
  CreatePressureEvent(int64_t timestamp, uint32_t expires, uint16_t pressureValue) {
    auto pressure = std::make_unique<Pinetime::Controllers::WeatherData::Pressure>();
    pressure->timestamp = timestamp;
    pressure->eventType = Pinetime::Controllers::WeatherData::eventtype::Pressure;
    pressure->expires = expires;
    pressure->pressure = pressureValue;
    return pressure;
  }

  std::unique_ptr<Pinetime::Controllers::WeatherData::Location> CreateLocationEvent(int64_t timestamp,
                                                                                    uint32_t expires,
                                                                                    const std::string& locationValue,
                                                                                    int16_t altitude,
                                                                                    int32_t latitude,
                                                                                    int32_t longitude) {
    auto location = std::make_unique<Pinetime::Controllers::WeatherData::Location>();
    location->timestamp = timestamp;
    location->eventType = Pinetime::Controllers::WeatherData::eventtype::Location;
    location->expires = expires;
    location->location = locationValue;
    location->altitude = altitude;
    location->latitude = latitude;
    location->longitude = longitude;
    return location;
  }

  std::unique_ptr<Pinetime::Controllers::WeatherData::Clouds> CreateCloudsEvent(int64_t timestamp, uint32_t expires, uint8_t amount) {
    auto clouds = std::make_unique<Pinetime::Controllers::WeatherData::Clouds>();
    clouds->timestamp = timestamp;
    clouds->eventType = Pinetime::Controllers::WeatherData::eventtype::Clouds;
    clouds->expires = expires;
    clouds->amount = amount;
    return clouds;
  }

  std::unique_ptr<Pinetime::Controllers::WeatherData::Humidity>
  CreateHumidityEvent(int64_t timestamp, uint32_t expires, uint8_t humidityValue) {
    auto humidity = std::make_unique<Pinetime::Controllers::WeatherData::Humidity>();
    humidity->timestamp = timestamp;
    humidity->eventType = Pinetime::Controllers::WeatherData::eventtype::Humidity;
    humidity->expires = expires;
    humidity->humidity = humidityValue;
    return humidity;
  }

  template <typename T>
  std::optional<T> Get(QCBORDecodeContext* /*context*/, const char* /*field*/) = delete;

  template <>
  std::optional<int64_t> Get<int64_t>(QCBORDecodeContext* context, const char* field) {
    int64_t tmp = 0;
    QCBORDecode_GetInt64InMapSZ(context, field, &tmp);
    if (QCBORDecode_GetError(context) != QCBOR_SUCCESS) {
      return {};
    }
    return {tmp};
  }

  template <>
  std::optional<uint32_t> Get<uint32_t>(QCBORDecodeContext* context, const char* field) {
    int64_t tmp = 0;
    QCBORDecode_GetInt64InMapSZ(context, field, &tmp);
    if (QCBORDecode_GetError(context) != QCBOR_SUCCESS || tmp < 0 || tmp > 4294967295) {
      return {};
    }
    return {static_cast<uint32_t>(tmp)};
  }

  template <>
  std::optional<int32_t> Get<int32_t>(QCBORDecodeContext* context, const char* field) {
    int64_t tmp = 0;
    QCBORDecode_GetInt64InMapSZ(context, field, &tmp);
    if (QCBORDecode_GetError(context) != QCBOR_SUCCESS || tmp < -2147483648 || tmp >= 2147483647) {
      return {};
    }
    return {static_cast<int32_t>(tmp)};
  }

  template <>
  std::optional<uint16_t> Get<uint16_t>(QCBORDecodeContext* context, const char* field) {
    int64_t tmp = 0;
    QCBORDecode_GetInt64InMapSZ(context, field, &tmp);
    if (QCBORDecode_GetError(context) != QCBOR_SUCCESS || tmp < 0 || tmp > 65535) {
      return {};
    }
    return {static_cast<uint16_t>(tmp)};
  }

  template <>
  std::optional<int16_t> Get<int16_t>(QCBORDecodeContext* context, const char* field) {
    int64_t tmp = 0;
    QCBORDecode_GetInt64InMapSZ(context, field, &tmp);
    if (QCBORDecode_GetError(context) != QCBOR_SUCCESS || tmp < -32768 || tmp >= 32767) {
      return {};
    }
    return {static_cast<int16_t>(tmp)};
  }

  template <>
  std::optional<uint8_t> Get<uint8_t>(QCBORDecodeContext* context, const char* field) {
    int64_t tmp = 0;
    QCBORDecode_GetInt64InMapSZ(context, field, &tmp);
    if (QCBORDecode_GetError(context) != QCBOR_SUCCESS || tmp < 0 || tmp > 255) {
      return {};
    }
    return {static_cast<uint8_t>(tmp)};
  }

  template <>
  std::optional<Pinetime::Controllers::WeatherData::eventtype>
  Get<Pinetime::Controllers::WeatherData::eventtype>(QCBORDecodeContext* context, const char* field) {
    int64_t tmp = 0;
    QCBORDecode_GetInt64InMapSZ(context, field, &tmp);
    if (QCBORDecode_GetError(context) != QCBOR_SUCCESS || tmp < 0 ||
        tmp >= static_cast<int64_t>(Pinetime::Controllers::WeatherData::eventtype::Length)) {
      return {};
    }
    return {static_cast<Pinetime::Controllers::WeatherData::eventtype>(tmp)};
  }

  template <>
  std::optional<std::string> Get<std::string>(QCBORDecodeContext* context, const char* field) {
    UsefulBufC stringBuf;
    QCBORDecode_GetTextStringInMapSZ(context, field, &stringBuf);
    if (UsefulBuf_IsNULLOrEmptyC(stringBuf) != 0) {
      return {};
    }
    return {std::string(static_cast<const char*>(stringBuf.ptr), stringBuf.len)};
  }

  template <>
  std::optional<Pinetime::Controllers::WeatherData::obscurationtype>
  Get<Pinetime::Controllers::WeatherData::obscurationtype>(QCBORDecodeContext* context, const char* field) {
    int64_t tmp = 0;
    QCBORDecode_GetInt64InMapSZ(context, field, &tmp);
    if (QCBORDecode_GetError(context) != QCBOR_SUCCESS || tmp < 0 ||
        tmp >= static_cast<int64_t>(Pinetime::Controllers::WeatherData::obscurationtype::Length)) {
      return {};
    }
    return {static_cast<Pinetime::Controllers::WeatherData::obscurationtype>(tmp)};
  }

  template <>
  std::optional<Pinetime::Controllers::WeatherData::precipitationtype>
  Get<Pinetime::Controllers::WeatherData::precipitationtype>(QCBORDecodeContext* context, const char* field) {
    int64_t tmp = 0;
    QCBORDecode_GetInt64InMapSZ(context, field, &tmp);
    if (QCBORDecode_GetError(context) != QCBOR_SUCCESS || tmp < 0 ||
        tmp >= static_cast<int64_t>(Pinetime::Controllers::WeatherData::precipitationtype::Length)) {
      return {};
    }
    return {static_cast<Pinetime::Controllers::WeatherData::precipitationtype>(tmp)};
  }

  template <>
  std::optional<Pinetime::Controllers::WeatherData::specialtype>
  Get<Pinetime::Controllers::WeatherData::specialtype>(QCBORDecodeContext* context, const char* field) {
    int64_t tmp = 0;
    QCBORDecode_GetInt64InMapSZ(context, field, &tmp);
    if (QCBORDecode_GetError(context) != QCBOR_SUCCESS || tmp < 0 ||
        tmp >= static_cast<int64_t>(Pinetime::Controllers::WeatherData::specialtype::Length)) {
      return {};
    }
    return {static_cast<Pinetime::Controllers::WeatherData::specialtype>(tmp)};
  }
}

int WeatherCallback(uint16_t /*connHandle*/, uint16_t /*attrHandle*/, struct ble_gatt_access_ctxt* ctxt, void* arg) {
  return static_cast<Pinetime::Controllers::WeatherService*>(arg)->OnCommand(ctxt);
}

namespace Pinetime {
  namespace Controllers {
    WeatherService::WeatherService(const DateTime& dateTimeController) : dateTimeController(dateTimeController) {
      nullHeader = &nullTimelineheader;
      nullTimelineheader->timestamp = 0;
    }

    void WeatherService::Init() {
      uint8_t res = 0;
      res = ble_gatts_count_cfg(serviceDefinition);
      ASSERT(res == 0);

      res = ble_gatts_add_svcs(serviceDefinition);
      ASSERT(res == 0);
    }

    int WeatherService::OnCommand(struct ble_gatt_access_ctxt* ctxt) {
      if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
        const uint8_t packetLen = OS_MBUF_PKTLEN(ctxt->om); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (packetLen <= 0) {
          return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        }
        // Decode
        QCBORDecodeContext decodeContext;
        UsefulBufC encodedCbor = {ctxt->om->om_data, OS_MBUF_PKTLEN(ctxt->om)}; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        QCBORDecode_Init(&decodeContext, encodedCbor, QCBOR_DECODE_MODE_NORMAL);
        // KINDLY provide us a fixed-length map
        QCBORDecode_EnterMap(&decodeContext, nullptr);
        // Always encodes to the smallest number of bytes based on the value
        auto optTimestamp = Get<int64_t>(&decodeContext, "Timestamp");
        if (!optTimestamp) {
          CleanUpQcbor(&decodeContext);
          return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        }

        auto optExpires = Get<uint32_t>(&decodeContext, "Expires");
        if (!optExpires) {
          CleanUpQcbor(&decodeContext);
          return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        }

        auto optEventType = Get<Pinetime::Controllers::WeatherData::eventtype>(&decodeContext, "EventType");
        if (!optEventType) {
          CleanUpQcbor(&decodeContext);
          return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        }

        switch (*optEventType) {
          case WeatherData::eventtype::AirQuality:
            if constexpr (EnableAirQuality) {
              auto optPolluter = Get<std::string>(&decodeContext, "Polluter");
              if (!optPolluter) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }

              auto optAmount = Get<uint32_t>(&decodeContext, "Amount");
              if (!optAmount) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }

              auto airQuality = CreateAirQualityEvent(*optTimestamp, *optExpires, *optPolluter, *optAmount);
              if (!AddEventToTimeline(std::move(airQuality))) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }
            }
            break;
          case WeatherData::eventtype::Obscuration:
            if constexpr (EnableObscuration) {
              auto optType = Get<WeatherData::obscurationtype>(&decodeContext, "Type");
              if (!optType) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }

              auto optAmount = Get<uint16_t>(&decodeContext, "Amount");
              if (!optAmount) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }

              auto obscuration = CreateObscurationEvent(*optTimestamp, *optExpires, *optAmount);
              if (!AddEventToTimeline(std::move(obscuration))) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }
            }
            break;
          case WeatherData::eventtype::Precipitation:
            if constexpr (EnablePrecipitation) {
              auto optType = Get<WeatherData::precipitationtype>(&decodeContext, "Type");
              if (optType) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }

              auto optAmount = Get<uint8_t>(&decodeContext, "Amount");
              if (!optAmount) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }

              auto precipitation = CreatePrecipitationEvent(*optTimestamp, *optExpires, *optType, *optAmount);
              if (!AddEventToTimeline(std::move(precipitation))) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }
            }
            break;
          case WeatherData::eventtype::Wind:
            if constexpr (EnableWind) {
              auto optMin = Get<uint8_t>(&decodeContext, "SpeedMin");
              if (!optMin) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }

              auto optMax = Get<uint8_t>(&decodeContext, "SpeedMax");
              if (!optMax) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }

              auto optDMin = Get<uint8_t>(&decodeContext, "DirectionMin");
              if (!optDMin) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }

              auto optDMax = Get<uint8_t>(&decodeContext, "DirectionMax");
              if (!optDMax) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }

              auto wind = CreateWindEvent(*optTimestamp, *optExpires, *optMin, *optMax, *optDMin, *optDMax);
              if (!AddEventToTimeline(std::move(wind))) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }
            }
            break;
          case WeatherData::eventtype::Temperature:
            if constexpr (EnableTemperature) {
              auto optTemperature = Get<int16_t>(&decodeContext, "Temperature");
              if (!optTemperature) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }

              auto optDewPoint = Get<int16_t>(&decodeContext, "DewPoint");
              if (!optDewPoint) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }

              auto temperature =
                CreateTemperatureEvent(*optTimestamp, *optExpires, *optTemperature, *optDewPoint);
              if (!AddEventToTimeline(std::move(temperature))) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }
            }
            break;
          case WeatherData::eventtype::Special:
            if constexpr (EnableSpecial) {
              auto optType = Get<WeatherData::specialtype>(&decodeContext, "Type");
              if (!optType) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }

              auto special = CreateSpecialEvent(*optTimestamp, *optExpires, *optType);
              if (!AddEventToTimeline(std::move(special))) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }
            }
            break;
          case WeatherData::eventtype::Pressure:
            if constexpr (EnablePressure) {
              auto optPressure = Get<uint16_t>(&decodeContext, "Pressure");
              if (!optPressure) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }

              auto pressure = CreatePressureEvent(*optTimestamp, *optExpires, *optPressure);
              if (!AddEventToTimeline(std::move(pressure))) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }
            }
            break;
          case WeatherData::eventtype::Location:
            if constexpr (EnableLocation) {
              auto optLocation = Get<std::string>(&decodeContext, "Location");
              if (!optLocation) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }

              auto optAltitude = Get<int16_t>(&decodeContext, "Altitude");
              if (!optAltitude) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }

              auto optLatitude = Get<int32_t>(&decodeContext, "Latitude");
              if (!optLatitude) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }

              auto optLongitude = Get<int32_t>(&decodeContext, "Longitude");
              if (!optLongitude) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }

              auto location = CreateLocationEvent(*optTimestamp, *optExpires, *optLocation, *optAltitude, *optLatitude, *optLongitude);
              if (!AddEventToTimeline(std::move(location))) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }
            }
            break;
          case WeatherData::eventtype::Clouds:
            if constexpr (EnableClouds) {
              auto optAmount = Get<uint8_t>(&decodeContext, "Amount");
              if (!optAmount) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }

              auto clouds = CreateCloudsEvent(*optTimestamp, *optExpires, *optAmount);
              if (!AddEventToTimeline(std::move(clouds))) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }
            }
            break;
          case WeatherData::eventtype::Humidity:
            if constexpr (EnableHumidity) {
              auto optType = Get<uint8_t>(&decodeContext, "Humidity");
              if (!optType) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }

              auto humidity = CreateHumidityEvent(*optTimestamp, *optExpires, *optType);
              if (!AddEventToTimeline(std::move(humidity))) {
                CleanUpQcbor(&decodeContext);
                return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
              }
            }
            break;
          default: {
            CleanUpQcbor(&decodeContext);
            return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
          }
        }

        QCBORDecode_ExitMap(&decodeContext);
        GetTimelineLength();
        TidyTimeline();

        if (QCBORDecode_Finish(&decodeContext) != QCBOR_SUCCESS) {
          return BLE_ATT_ERR_INSUFFICIENT_RES;
        }
      } else if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
        // Encode
        uint8_t buffer[64];
        QCBOREncodeContext encodeContext;
        /* TODO: This is very much still a test endpoint
         *  it needs a characteristic UUID check
         *  and actual implementations that show
         *  what actually has to be read.
         *  WARN: Consider commands not part of the API for now!
         */
        QCBOREncode_Init(&encodeContext, UsefulBuf_FROM_BYTE_ARRAY(buffer));
        QCBOREncode_OpenMap(&encodeContext);
        QCBOREncode_AddTextToMap(&encodeContext, "test", UsefulBuf_FROM_SZ_LITERAL("test"));
        QCBOREncode_AddInt64ToMap(&encodeContext, "test", 1ul);
        QCBOREncode_CloseMap(&encodeContext);

        UsefulBufC encodedEvent;
        auto uErr = QCBOREncode_Finish(&encodeContext, &encodedEvent);
        if (uErr != 0) {
          return BLE_ATT_ERR_INSUFFICIENT_RES;
        }
        auto res = os_mbuf_append(ctxt->om, &buffer, sizeof(buffer));
        if (res == 0) {
          return BLE_ATT_ERR_INSUFFICIENT_RES;
        }

        return 0;
      }
      return 0;
    }

    std::unique_ptr<WeatherData::Clouds>& WeatherService::GetCurrentClouds() {
      uint64_t currentTimestamp = GetCurrentUnixTimestamp();
      for (auto&& header : this->timeline) {
        if (header->eventType == WeatherData::eventtype::Clouds && IsEventStillValid(header, currentTimestamp)) {
          return reinterpret_cast<std::unique_ptr<WeatherData::Clouds>&>(header);
        }
      }

      return reinterpret_cast<std::unique_ptr<WeatherData::Clouds>&>(*this->nullHeader);
    }

    std::unique_ptr<WeatherData::Obscuration>& WeatherService::GetCurrentObscuration() {
      uint64_t currentTimestamp = GetCurrentUnixTimestamp();
      for (auto&& header : this->timeline) {
        if (header->eventType == WeatherData::eventtype::Obscuration && IsEventStillValid(header, currentTimestamp)) {
          return reinterpret_cast<std::unique_ptr<WeatherData::Obscuration>&>(header);
        }
      }

      return reinterpret_cast<std::unique_ptr<WeatherData::Obscuration>&>(*this->nullHeader);
    }

    std::unique_ptr<WeatherData::Precipitation>& WeatherService::GetCurrentPrecipitation() {
      uint64_t currentTimestamp = GetCurrentUnixTimestamp();
      for (auto&& header : this->timeline) {
        if (header->eventType == WeatherData::eventtype::Precipitation && IsEventStillValid(header, currentTimestamp)) {
          return reinterpret_cast<std::unique_ptr<WeatherData::Precipitation>&>(header);
        }
      }

      return reinterpret_cast<std::unique_ptr<WeatherData::Precipitation>&>(*this->nullHeader);
    }

    std::unique_ptr<WeatherData::Wind>& WeatherService::GetCurrentWind() {
      uint64_t currentTimestamp = GetCurrentUnixTimestamp();
      for (auto&& header : this->timeline) {
        if (header->eventType == WeatherData::eventtype::Wind && IsEventStillValid(header, currentTimestamp)) {
          return reinterpret_cast<std::unique_ptr<WeatherData::Wind>&>(header);
        }
      }

      return reinterpret_cast<std::unique_ptr<WeatherData::Wind>&>(*this->nullHeader);
    }

    std::unique_ptr<WeatherData::Temperature>& WeatherService::GetCurrentTemperature() {
      uint64_t currentTimestamp = GetCurrentUnixTimestamp();
      for (auto&& header : this->timeline) {
        if (header->eventType == WeatherData::eventtype::Temperature && IsEventStillValid(header, currentTimestamp)) {
          return reinterpret_cast<std::unique_ptr<WeatherData::Temperature>&>(header);
        }
      }

      return reinterpret_cast<std::unique_ptr<WeatherData::Temperature>&>(*this->nullHeader);
    }

    std::unique_ptr<WeatherData::Humidity>& WeatherService::GetCurrentHumidity() {
      uint64_t currentTimestamp = GetCurrentUnixTimestamp();
      for (auto&& header : this->timeline) {
        if (header->eventType == WeatherData::eventtype::Humidity && IsEventStillValid(header, currentTimestamp)) {
          return reinterpret_cast<std::unique_ptr<WeatherData::Humidity>&>(header);
        }
      }

      return reinterpret_cast<std::unique_ptr<WeatherData::Humidity>&>(*this->nullHeader);
    }

    std::unique_ptr<WeatherData::Pressure>& WeatherService::GetCurrentPressure() {
      uint64_t currentTimestamp = GetCurrentUnixTimestamp();
      for (auto&& header : this->timeline) {
        if (header->eventType == WeatherData::eventtype::Pressure && IsEventStillValid(header, currentTimestamp)) {
          return reinterpret_cast<std::unique_ptr<WeatherData::Pressure>&>(header);
        }
      }

      return reinterpret_cast<std::unique_ptr<WeatherData::Pressure>&>(*this->nullHeader);
    }

    std::unique_ptr<WeatherData::Location>& WeatherService::GetCurrentLocation() {
      uint64_t currentTimestamp = GetCurrentUnixTimestamp();
      for (auto&& header : this->timeline) {
        if (header->eventType == WeatherData::eventtype::Location && IsEventStillValid(header, currentTimestamp)) {
          return reinterpret_cast<std::unique_ptr<WeatherData::Location>&>(header);
        }
      }

      return reinterpret_cast<std::unique_ptr<WeatherData::Location>&>(*this->nullHeader);
    }

    std::unique_ptr<WeatherData::AirQuality>& WeatherService::GetCurrentQuality() {
      uint64_t currentTimestamp = GetCurrentUnixTimestamp();
      for (auto&& header : this->timeline) {
        if (header->eventType == WeatherData::eventtype::AirQuality && IsEventStillValid(header, currentTimestamp)) {
          return reinterpret_cast<std::unique_ptr<WeatherData::AirQuality>&>(header);
        }
      }

      return reinterpret_cast<std::unique_ptr<WeatherData::AirQuality>&>(*this->nullHeader);
    }

    size_t WeatherService::GetTimelineLength() const {
      return timeline.size();
    }

    bool WeatherService::AddEventToTimeline(std::unique_ptr<WeatherData::TimelineHeader> event) {
      if (timeline.size() == maxNbElements) {
        return false;
      }

      timeline.push_back(std::move(event));
      return true;
    }

    bool WeatherService::HasTimelineEventOfType(const WeatherData::eventtype type) const {
      uint64_t currentTimestamp = GetCurrentUnixTimestamp();
      for (auto&& header : timeline) {
        if (header->eventType == type && IsEventStillValid(header, currentTimestamp)) {
          return true;
        }
      }
      return false;
    }

    void WeatherService::TidyTimeline() {
      uint64_t timeCurrent = GetCurrentUnixTimestamp();
      timeline.erase(std::remove_if(std::begin(timeline),
                                    std::end(timeline),
                                    [&](std::unique_ptr<WeatherData::TimelineHeader> const& header) {
                                      return !IsEventStillValid(header, timeCurrent);
                                    }),
                     std::end(timeline));

      std::sort(std::begin(timeline), std::end(timeline), CompareTimelineEvents);
    }

    bool WeatherService::CompareTimelineEvents(const std::unique_ptr<WeatherData::TimelineHeader>& first,
                                               const std::unique_ptr<WeatherData::TimelineHeader>& second) {
      return first->timestamp > second->timestamp;
    }

    bool WeatherService::IsEventStillValid(const std::unique_ptr<WeatherData::TimelineHeader>& uniquePtr, const uint64_t timestamp) {
      // Not getting timestamp in isEventStillValid for more speed
      return uniquePtr->timestamp + uniquePtr->expires >= timestamp;
    }

    uint64_t WeatherService::GetCurrentUnixTimestamp() const {
      return std::chrono::duration_cast<std::chrono::seconds>(dateTimeController.CurrentDateTime().time_since_epoch()).count();
    }

    int16_t WeatherService::GetTodayMinTemp() const {
      uint64_t currentTimestamp = GetCurrentUnixTimestamp();
      uint64_t currentDayEnd = currentTimestamp + ((24 - dateTimeController.Hours()) * 60 * 60) +
                               ((60 - dateTimeController.Minutes()) * 60) + (60 - dateTimeController.Seconds());
      uint64_t currentDayStart = currentDayEnd - 86400;
      int16_t result = -32768;
      for (auto&& header : this->timeline) {
        if (header->eventType == WeatherData::eventtype::Temperature && header->timestamp >= currentDayStart &&
            header->timestamp < currentDayEnd &&
            reinterpret_cast<const std::unique_ptr<WeatherData::Temperature>&>(header)->temperature != -32768) {
          int16_t temperature = reinterpret_cast<const std::unique_ptr<WeatherData::Temperature>&>(header)->temperature;
          if (result == -32768) {
            result = temperature;
          } else if (result > temperature) {
            result = temperature;
          } else {
            // The temperature in this item is higher than the lowest we've found
          }
        }
      }

      return result;
    }

    int16_t WeatherService::GetTodayMaxTemp() const {
      uint64_t currentTimestamp = GetCurrentUnixTimestamp();
      uint64_t currentDayEnd = currentTimestamp + ((24 - dateTimeController.Hours()) * 60 * 60) +
                               ((60 - dateTimeController.Minutes()) * 60) + (60 - dateTimeController.Seconds());
      uint64_t currentDayStart = currentDayEnd - 86400;
      int16_t result = -32768;
      for (auto&& header : this->timeline) {
        if (header->eventType == WeatherData::eventtype::Temperature && header->timestamp >= currentDayStart &&
            header->timestamp < currentDayEnd &&
            reinterpret_cast<const std::unique_ptr<WeatherData::Temperature>&>(header)->temperature != -32768) {
          int16_t temperature = reinterpret_cast<const std::unique_ptr<WeatherData::Temperature>&>(header)->temperature;
          if (result == -32768) {
            result = temperature;
          } else if (result < temperature) {
            result = temperature;
          } else {
            // The temperature in this item is lower than the highest we've found
          }
        }
      }

      return result;
    }

    void WeatherService::CleanUpQcbor(QCBORDecodeContext* decodeContext) {
      QCBORDecode_ExitMap(decodeContext);
      QCBORDecode_Finish(decodeContext);
    }
  }
}
