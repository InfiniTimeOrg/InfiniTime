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
#include <qcbor/qcbor_spiffy_decode.h>
#include "WeatherService.h"
#include "libs/QCBOR/inc/qcbor/qcbor.h"
#include "systemtask/SystemTask.h"

int WeatherCallback(uint16_t connHandle, uint16_t attrHandle, struct ble_gatt_access_ctxt* ctxt, void* arg) {
  return static_cast<Pinetime::Controllers::WeatherService*>(arg)->OnCommand(connHandle, attrHandle, ctxt);
}

namespace Pinetime {
  namespace Controllers {
    WeatherService::WeatherService(System::SystemTask& system, DateTime& dateTimeController)
      : system(system), dateTimeController(dateTimeController) {
    }

    void WeatherService::Init() {
      uint8_t res = 0;
      res = ble_gatts_count_cfg(serviceDefinition);
      ASSERT(res == 0);

      res = ble_gatts_add_svcs(serviceDefinition);
      ASSERT(res == 0);
    }

    int WeatherService::OnCommand(uint16_t connHandle, uint16_t attrHandle, struct ble_gatt_access_ctxt* ctxt) {
      // TODO: Detect control messages
      if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
        const auto packetLen = OS_MBUF_PKTLEN(ctxt->om);
        if (packetLen <= 0) {
          return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        }
        // Decode
        QCBORDecodeContext decodeContext;
        UsefulBufC encodedCbor;
        // TODO: Check, uninit fine?

        QCBORDecode_Init(&decodeContext, encodedCbor, QCBOR_DECODE_MODE_NORMAL);
        QCBORDecode_EnterMap(&decodeContext, nullptr);
        // Always encodes to the smallest number of bytes based on the value
        int64_t tmpVersion = 0;
        QCBORDecode_GetInt64InMapSZ(&decodeContext, "Version", &tmpVersion);
        if (tmpVersion != 1) {
          // TODO: Return better error?
          return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        }
        int64_t tmpTimestamp = 0;
        QCBORDecode_GetInt64InMapSZ(&decodeContext, "Timestamp", &tmpTimestamp);
        int64_t tmpExpires = 0;
        QCBORDecode_GetInt64InMapSZ(&decodeContext, "Expires", &tmpExpires);
        if (tmpExpires < 0 || tmpExpires > 4294967295) {
          // TODO: Return better error?
          return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        }
        int64_t tmpEventType = 0;
        QCBORDecode_GetInt64InMapSZ(&decodeContext, "EventType", &tmpEventType);
        if (tmpEventType < 0 || tmpEventType > static_cast<int64_t>(WeatherData::eventtype::Length)) {
          // TODO: Return better error?
          return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        }

        switch (static_cast<WeatherData::eventtype>(tmpEventType)) {
          // TODO: Populate
          case WeatherData::eventtype::AirQuality: {
            std::unique_ptr<WeatherData::AirQuality> airquality = std::make_unique<WeatherData::AirQuality>();
            airquality->timestamp = tmpTimestamp;
            airquality->eventType = static_cast<WeatherData::eventtype>(tmpEventType);
            airquality->expires = tmpExpires;
            UsefulBufC String;
            QCBORDecode_GetTextStringInMapSZ(&decodeContext, "Polluter", &String);
            if (UsefulBuf_IsNULLOrEmptyC(String) != 0) {
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            airquality->polluter = std::make_unique<std::string>(static_cast<const char*>(String.ptr), String.len);
            int64_t tmpAmount = 0;
            QCBORDecode_GetInt64InMapSZ(&decodeContext, "Amount", &tmpAmount);
            if (tmpAmount < 0 || tmpAmount > 4294967295) {
              return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            }
            airquality->amount = tmpAmount;
            timeline.push_back(std::move(airquality));
            break;
          }
          case WeatherData::eventtype::Obscuration: {
            std::unique_ptr<WeatherData::Obscuration> obscuration = std::make_unique<WeatherData::Obscuration>();
            obscuration->timestamp = tmpTimestamp;
            obscuration->eventType = static_cast<WeatherData::eventtype>(tmpEventType);
            obscuration->expires = tmpExpires;

            timeline.push_back(std::move(obscuration));
            break;
          }
          case WeatherData::eventtype::Precipitation: {
            std::unique_ptr<WeatherData::Precipitation> precipitation = std::make_unique<WeatherData::Precipitation>();
            precipitation->timestamp = tmpTimestamp;
            precipitation->eventType = static_cast<WeatherData::eventtype>(tmpEventType);
            precipitation->expires = tmpExpires;
            timeline.push_back(std::move(precipitation));
            break;
          }
          case WeatherData::eventtype::Wind: {
            std::unique_ptr<WeatherData::Wind> wind = std::make_unique<WeatherData::Wind>();
            wind->timestamp = tmpTimestamp;
            wind->eventType = static_cast<WeatherData::eventtype>(tmpEventType);
            wind->expires = tmpExpires;
            timeline.push_back(std::move(wind));
            break;
          }
          case WeatherData::eventtype::Temperature: {
            std::unique_ptr<WeatherData::Temperature> temperature = std::make_unique<WeatherData::Temperature>();
            temperature->timestamp = tmpTimestamp;
            temperature->eventType = static_cast<WeatherData::eventtype>(tmpEventType);
            temperature->expires = tmpExpires;
            timeline.push_back(std::move(temperature));
            break;
          }
          case WeatherData::eventtype::Special: {
            std::unique_ptr<WeatherData::Special> special = std::make_unique<WeatherData::Special>();
            special->timestamp = tmpTimestamp;
            special->eventType = static_cast<WeatherData::eventtype>(tmpEventType);
            special->expires = tmpExpires;
            timeline.push_back(std::move(special));
            break;
          }
          case WeatherData::eventtype::Pressure: {
            std::unique_ptr<WeatherData::Pressure> pressure = std::make_unique<WeatherData::Pressure>();
            pressure->timestamp = tmpTimestamp;
            pressure->eventType = static_cast<WeatherData::eventtype>(tmpEventType);
            pressure->expires = tmpExpires;
            timeline.push_back(std::move(pressure));
            break;
          }
          case WeatherData::eventtype::Location: {
            std::unique_ptr<WeatherData::Location> location = std::make_unique<WeatherData::Location>();
            location->timestamp = tmpTimestamp;
            location->eventType = static_cast<WeatherData::eventtype>(tmpEventType);
            location->expires = tmpExpires;
            timeline.push_back(std::move(location));
            break;
          }
          case WeatherData::eventtype::Clouds: {
            std::unique_ptr<WeatherData::Clouds> clouds = std::make_unique<WeatherData::Clouds>();
            clouds->timestamp = tmpTimestamp;
            clouds->eventType = static_cast<WeatherData::eventtype>(tmpEventType);
            clouds->expires = tmpExpires;
            timeline.push_back(std::move(clouds));
            break;
          }
          default: {
            break;
          }
        }

        GetCurrentPressure();
        TidyTimeline();
        GetTimelineLength();
        QCBORDecode_ExitMap(&decodeContext);

        auto uErr = QCBORDecode_Finish(&decodeContext);
        if (uErr != 0) {
          return BLE_ATT_ERR_INSUFFICIENT_RES;
        }
      } else if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
        // Encode
        uint8_t buffer[64];
        QCBOREncodeContext encodeContext;
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

    WeatherData::Location WeatherService::GetCurrentLocation() const {
      return WeatherData::Location();
    }

    WeatherData::Clouds WeatherService::GetCurrentClouds() const {
      return WeatherData::Clouds();
    }

    WeatherData::Obscuration WeatherService::GetCurrentObscuration() const {
      return WeatherData::Obscuration();
    }

    WeatherData::Precipitation WeatherService::GetCurrentPrecipitation() const {
      return WeatherData::Precipitation();
    }

    WeatherData::Wind WeatherService::GetCurrentWind() const {
      return WeatherData::Wind();
    }

    WeatherData::Temperature WeatherService::GetCurrentTemperature() const {
      return WeatherData::Temperature();
    }

    WeatherData::Humidity WeatherService::GetCurrentHumidity() const {
      return WeatherData::Humidity();
    }

    WeatherData::Pressure WeatherService::GetCurrentPressure() const {
      uint64_t currentTimestamp = GetCurrentUnixTimestamp();
      for (auto&& header : timeline) {
        if (header->eventType == WeatherData::eventtype::Pressure && header->timestamp + header->expires <= currentTimestamp) {
          return WeatherData::Pressure();
        }
      }
      return WeatherData::Pressure();
    }

    WeatherData::AirQuality WeatherService::GetCurrentQuality() const {
      return WeatherData::AirQuality();
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
        if (header->eventType == type && header->timestamp + header->expires <= currentTimestamp) {
          // TODO: Check if its currently valid
          return true;
        }
      }
      return false;
    }

    void WeatherService::TidyTimeline() {
      uint64_t timeCurrent = 0;
      timeline.erase(std::remove_if(std::begin(timeline),
                                    std::end(timeline),
                                    [&](std::unique_ptr<WeatherData::TimelineHeader> const& header) {
                                      return header->timestamp + header->expires > timeCurrent;
                                    }),
                     std::end(timeline));

      std::sort(std::begin(timeline), std::end(timeline), CompareTimelineEvents);
    }

    bool WeatherService::CompareTimelineEvents(const std::unique_ptr<WeatherData::TimelineHeader>& first,
                                               const std::unique_ptr<WeatherData::TimelineHeader>& second) {
      return first->timestamp > second->timestamp;
    }

    uint64_t WeatherService::GetCurrentUnixTimestamp() const {
      return std::chrono::duration_cast<std::chrono::seconds>(dateTimeController.CurrentDateTime().time_since_epoch()).count();
    }
  }
}
