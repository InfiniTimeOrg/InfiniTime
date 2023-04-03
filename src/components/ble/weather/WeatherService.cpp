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
        int64_t tmpTimestamp = 0;
        QCBORDecode_GetInt64InMapSZ(&decodeContext, "Timestamp", &tmpTimestamp);
        if (QCBORDecode_GetError(&decodeContext) != QCBOR_SUCCESS) {
          CleanUpQcbor(&decodeContext);
          return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        }
        int64_t tmpExpires = 0;
        QCBORDecode_GetInt64InMapSZ(&decodeContext, "Expires", &tmpExpires);
        if (QCBORDecode_GetError(&decodeContext) != QCBOR_SUCCESS || tmpExpires < 0 || tmpExpires > 4294967295) {
          CleanUpQcbor(&decodeContext);
          return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        }
        int64_t tmpEventType = 0;
        QCBORDecode_GetInt64InMapSZ(&decodeContext, "EventType", &tmpEventType);
        if (QCBORDecode_GetError(&decodeContext) != QCBOR_SUCCESS || tmpEventType < 0 ||
            tmpEventType >= static_cast<int64_t>(WeatherData::eventtype::Length)) {
          CleanUpQcbor(&decodeContext);
          return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        }

        switch (static_cast<WeatherData::eventtype>(tmpEventType)) {
          case WeatherData::eventtype::AirQuality: {
            std::unique_ptr<WeatherData::AirQuality> airquality = std::make_unique<WeatherData::AirQuality>();
            airquality->timestamp = tmpTimestamp;
            airquality->eventType = static_cast<WeatherData::eventtype>(tmpEventType);
            airquality->expires = tmpExpires;

            UsefulBufC stringBuf; // TODO: Everything ok with lifecycle here?
            QCBORDecode_GetTextStringInMapSZ(&decodeContext, "Polluter", &stringBuf);
            if (UsefulBuf_IsNULLOrEmptyC(stringBuf) != 0) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            airquality->polluter = std::string(static_cast<const char*>(stringBuf.ptr), stringBuf.len);

            int64_t tmpAmount = 0;
            QCBORDecode_GetInt64InMapSZ(&decodeContext, "Amount", &tmpAmount);
            if (tmpAmount < 0 || tmpAmount > 4294967295) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            airquality->amount = tmpAmount; // NOLINT(bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)

            if (!AddEventToTimeline(std::move(airquality))) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            break;
          }
          case WeatherData::eventtype::Obscuration: {
            std::unique_ptr<WeatherData::Obscuration> obscuration = std::make_unique<WeatherData::Obscuration>();
            obscuration->timestamp = tmpTimestamp;
            obscuration->eventType = static_cast<WeatherData::eventtype>(tmpEventType);
            obscuration->expires = tmpExpires;

            int64_t tmpType = 0;
            QCBORDecode_GetInt64InMapSZ(&decodeContext, "Type", &tmpType);
            if (tmpType < 0 || tmpType >= static_cast<int64_t>(WeatherData::obscurationtype::Length)) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            obscuration->type = static_cast<WeatherData::obscurationtype>(tmpType);

            int64_t tmpAmount = 0;
            QCBORDecode_GetInt64InMapSZ(&decodeContext, "Amount", &tmpAmount);
            if (tmpAmount < 0 || tmpAmount > 65535) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            obscuration->amount = tmpAmount; // NOLINT(bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)

            if (!AddEventToTimeline(std::move(obscuration))) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            break;
          }
          case WeatherData::eventtype::Precipitation: {
            std::unique_ptr<WeatherData::Precipitation> precipitation = std::make_unique<WeatherData::Precipitation>();
            precipitation->timestamp = tmpTimestamp;
            precipitation->eventType = static_cast<WeatherData::eventtype>(tmpEventType);
            precipitation->expires = tmpExpires;

            int64_t tmpType = 0;
            QCBORDecode_GetInt64InMapSZ(&decodeContext, "Type", &tmpType);
            if (tmpType < 0 || tmpType >= static_cast<int64_t>(WeatherData::precipitationtype::Length)) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            precipitation->type = static_cast<WeatherData::precipitationtype>(tmpType);

            int64_t tmpAmount = 0;
            QCBORDecode_GetInt64InMapSZ(&decodeContext, "Amount", &tmpAmount);
            if (tmpAmount < 0 || tmpAmount > 255) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            precipitation->amount = tmpAmount; // NOLINT(bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)

            if (!AddEventToTimeline(std::move(precipitation))) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            break;
          }
          case WeatherData::eventtype::Wind: {
            std::unique_ptr<WeatherData::Wind> wind = std::make_unique<WeatherData::Wind>();
            wind->timestamp = tmpTimestamp;
            wind->eventType = static_cast<WeatherData::eventtype>(tmpEventType);
            wind->expires = tmpExpires;

            int64_t tmpMin = 0;
            QCBORDecode_GetInt64InMapSZ(&decodeContext, "SpeedMin", &tmpMin);
            if (tmpMin < 0 || tmpMin > 255) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            wind->speedMin = tmpMin; // NOLINT(bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)

            int64_t tmpMax = 0;
            QCBORDecode_GetInt64InMapSZ(&decodeContext, "SpeedMin", &tmpMax);
            if (tmpMax < 0 || tmpMax > 255) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            wind->speedMax = tmpMax; // NOLINT(bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)

            int64_t tmpDMin = 0;
            QCBORDecode_GetInt64InMapSZ(&decodeContext, "DirectionMin", &tmpDMin);
            if (tmpDMin < 0 || tmpDMin > 255) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            wind->directionMin = tmpDMin; // NOLINT(bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)

            int64_t tmpDMax = 0;
            QCBORDecode_GetInt64InMapSZ(&decodeContext, "DirectionMax", &tmpDMax);
            if (tmpDMax < 0 || tmpDMax > 255) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            wind->directionMax = tmpDMax; // NOLINT(bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)

            if (!AddEventToTimeline(std::move(wind))) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            break;
          }
          case WeatherData::eventtype::Temperature: {
            std::unique_ptr<WeatherData::Temperature> temperature = std::make_unique<WeatherData::Temperature>();
            temperature->timestamp = tmpTimestamp;
            temperature->eventType = static_cast<WeatherData::eventtype>(tmpEventType);
            temperature->expires = tmpExpires;

            int64_t tmpTemperature = 0;
            QCBORDecode_GetInt64InMapSZ(&decodeContext, "Temperature", &tmpTemperature);
            if (tmpTemperature < -32768 || tmpTemperature > 32767) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            temperature->temperature =
              static_cast<int16_t>(tmpTemperature); // NOLINT(bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)

            int64_t tmpDewPoint = 0;
            QCBORDecode_GetInt64InMapSZ(&decodeContext, "DewPoint", &tmpDewPoint);
            if (tmpDewPoint < -32768 || tmpDewPoint > 32767) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            temperature->dewPoint =
              static_cast<int16_t>(tmpDewPoint); // NOLINT(bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)

            if (!AddEventToTimeline(std::move(temperature))) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            break;
          }
          case WeatherData::eventtype::Special: {
            std::unique_ptr<WeatherData::Special> special = std::make_unique<WeatherData::Special>();
            special->timestamp = tmpTimestamp;
            special->eventType = static_cast<WeatherData::eventtype>(tmpEventType);
            special->expires = tmpExpires;

            int64_t tmpType = 0;
            QCBORDecode_GetInt64InMapSZ(&decodeContext, "Type", &tmpType);
            if (tmpType < 0 || tmpType >= static_cast<int64_t>(WeatherData::specialtype::Length)) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            special->type = static_cast<WeatherData::specialtype>(tmpType);

            if (!AddEventToTimeline(std::move(special))) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            break;
          }
          case WeatherData::eventtype::Pressure: {
            std::unique_ptr<WeatherData::Pressure> pressure = std::make_unique<WeatherData::Pressure>();
            pressure->timestamp = tmpTimestamp;
            pressure->eventType = static_cast<WeatherData::eventtype>(tmpEventType);
            pressure->expires = tmpExpires;

            int64_t tmpPressure = 0;
            QCBORDecode_GetInt64InMapSZ(&decodeContext, "Pressure", &tmpPressure);
            if (tmpPressure < 0 || tmpPressure >= 65535) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            pressure->pressure = tmpPressure; // NOLINT(bugprone-narrowing-conversions,cppcoreguidelines-narrowing-conversions)

            if (!AddEventToTimeline(std::move(pressure))) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            break;
          }
          case WeatherData::eventtype::Location: {
            std::unique_ptr<WeatherData::Location> location = std::make_unique<WeatherData::Location>();
            location->timestamp = tmpTimestamp;
            location->eventType = static_cast<WeatherData::eventtype>(tmpEventType);
            location->expires = tmpExpires;

            UsefulBufC stringBuf; // TODO: Everything ok with lifecycle here?
            QCBORDecode_GetTextStringInMapSZ(&decodeContext, "Location", &stringBuf);
            if (UsefulBuf_IsNULLOrEmptyC(stringBuf) != 0) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            location->location = std::string(static_cast<const char*>(stringBuf.ptr), stringBuf.len);

            int64_t tmpAltitude = 0;
            QCBORDecode_GetInt64InMapSZ(&decodeContext, "Altitude", &tmpAltitude);
            if (tmpAltitude < -32768 || tmpAltitude >= 32767) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            location->altitude = static_cast<int16_t>(tmpAltitude);

            int64_t tmpLatitude = 0;
            QCBORDecode_GetInt64InMapSZ(&decodeContext, "Latitude", &tmpLatitude);
            if (tmpLatitude < -2147483648 || tmpLatitude >= 2147483647) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            location->latitude = static_cast<int32_t>(tmpLatitude);

            int64_t tmpLongitude = 0;
            QCBORDecode_GetInt64InMapSZ(&decodeContext, "Longitude", &tmpLongitude);
            if (tmpLongitude < -2147483648 || tmpLongitude >= 2147483647) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            location->latitude = static_cast<int32_t>(tmpLongitude);

            if (!AddEventToTimeline(std::move(location))) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            break;
          }
          case WeatherData::eventtype::Clouds: {
            std::unique_ptr<WeatherData::Clouds> clouds = std::make_unique<WeatherData::Clouds>();
            clouds->timestamp = tmpTimestamp;
            clouds->eventType = static_cast<WeatherData::eventtype>(tmpEventType);
            clouds->expires = tmpExpires;

            int64_t tmpAmount = 0;
            QCBORDecode_GetInt64InMapSZ(&decodeContext, "Amount", &tmpAmount);
            if (tmpAmount < 0 || tmpAmount > 255) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            clouds->amount = static_cast<uint8_t>(tmpAmount);

            if (!AddEventToTimeline(std::move(clouds))) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            break;
          }
          case WeatherData::eventtype::Humidity: {
            std::unique_ptr<WeatherData::Humidity> humidity = std::make_unique<WeatherData::Humidity>();
            humidity->timestamp = tmpTimestamp;
            humidity->eventType = static_cast<WeatherData::eventtype>(tmpEventType);
            humidity->expires = tmpExpires;

            int64_t tmpType = 0;
            QCBORDecode_GetInt64InMapSZ(&decodeContext, "Humidity", &tmpType);
            if (tmpType < 0 || tmpType >= 255) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            humidity->humidity = static_cast<uint8_t>(tmpType);

            if (!AddEventToTimeline(std::move(humidity))) {
              CleanUpQcbor(&decodeContext);
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            break;
          }
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
      if (timeline.size() == timeline.max_size()) {
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
