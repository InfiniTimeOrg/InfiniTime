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

int WeatherCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt, void* arg) {
  return static_cast<Pinetime::Controllers::WeatherService*>(arg)->OnCommand(conn_handle, attr_handle, ctxt);
}

namespace Pinetime {
  namespace Controllers {
    WeatherService::WeatherService(System::SystemTask& system, DateTime& dateTimeController)
      : system(system), dateTimeController(dateTimeController) {
    }

    void WeatherService::Init() {
      uint8_t res = 0;
      res = ble_gatts_count_cfg(serviceDefinition);
      ASSERT(res == 0)

      res = ble_gatts_add_svcs(serviceDefinition);
      ASSERT(res == 0);
    }

    int WeatherService::OnCommand(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt) {
      if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
        getCurrentPressure();
        tidyTimeline();
        getTimelineLength();
        const auto packetLen = OS_MBUF_PKTLEN(ctxt->om);
        if (packetLen <= 0) {
          return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
        }
        // Decode
        QCBORDecodeContext decodeContext;
        UsefulBufC EncodedCBOR;
        // TODO: Check uninit fine
        QCBORDecode_Init(&decodeContext, EncodedCBOR, QCBOR_DECODE_MODE_NORMAL);
        QCBORDecode_EnterMap(&decodeContext, nullptr);
        WeatherData::timelineheader timelineHeader {};
        // Always encodes to the smallest number of bytes based on the value
        QCBORDecode_GetInt64InMapSZ(&decodeContext, "Timestamp", reinterpret_cast<int64_t*>(&(timelineHeader.timestamp)));
        QCBORDecode_GetInt64InMapSZ(&decodeContext, "Expires", reinterpret_cast<int64_t*>(&(timelineHeader.expires)));
        QCBORDecode_GetInt64InMapSZ(&decodeContext, "EventType", reinterpret_cast<int64_t*>(&(timelineHeader.eventType)));
        switch (timelineHeader.eventType) {
            // TODO: Populate
          case WeatherData::eventtype::AirQuality: {
            break;
          }
          case WeatherData::eventtype::Obscuration: {
            break;
          }
          case WeatherData::eventtype::Precipitation: {
            break;
          }
          case WeatherData::eventtype::Wind: {
            break;
          }
          case WeatherData::eventtype::Temperature: {
            break;
          }
          case WeatherData::eventtype::Special: {
            break;
          }
          case WeatherData::eventtype::Pressure: {
            break;
          }
          case WeatherData::eventtype::Location: {
            break;
          }
          case WeatherData::eventtype::Clouds: {
            break;
          }
          default: {
            break;
          }
        }
        QCBORDecode_ExitMap(&decodeContext);

        auto uErr = QCBORDecode_Finish(&decodeContext);
        if (uErr != 0) {
          return BLE_ATT_ERR_INSUFFICIENT_RES;
        }
      } else if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
        // TODO: Detect control messages

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

    WeatherData::location WeatherService::getCurrentLocation() const {
      return WeatherData::location();
    }
    WeatherData::clouds WeatherService::getCurrentClouds() const {
      return WeatherData::clouds();
    }
    WeatherData::obscuration WeatherService::getCurrentObscuration() const {
      return WeatherData::obscuration();
    }
    WeatherData::precipitation WeatherService::getCurrentPrecipitation() const {
      return WeatherData::precipitation();
    }
    WeatherData::wind WeatherService::getCurrentWind() const {
      return WeatherData::wind();
    }
    WeatherData::temperature WeatherService::getCurrentTemperature() const {
      return WeatherData::temperature();
    }
    WeatherData::humidity WeatherService::getCurrentHumidity() const {
      return WeatherData::humidity();
    }
    WeatherData::pressure WeatherService::getCurrentPressure() const {
      uint64_t currentTimestamp = getCurrentUNIXTimestamp();
      for (auto&& header : timeline) {
        if (header->eventType == WeatherData::eventtype::Pressure && header->timestamp + header->expires <= currentTimestamp) {
          return WeatherData::pressure();
        }
      }
      return WeatherData::pressure();
    }

    WeatherData::airquality WeatherService::getCurrentQuality() const {
      return WeatherData::airquality();
    }

    size_t WeatherService::getTimelineLength() const {
      return timeline.size();
    }

    bool WeatherService::addEventToTimeline(std::unique_ptr<WeatherData::timelineheader> event) {
      if (timeline.size() == timeline.max_size()) {
        return false;
      }

      timeline.push_back(std::move(event));
      return true;
    }

    bool WeatherService::hasTimelineEventOfType(const WeatherData::eventtype type) const {
      uint64_t currentTimestamp = getCurrentUNIXTimestamp();
      for (auto&& header : timeline) {
        if (header->eventType == type && header->timestamp + header->expires <= currentTimestamp) {
          // TODO: Check if its currently valid
          return true;
        }
      }
      return false;
    }

    void WeatherService::tidyTimeline() {
      uint64_t timeCurrent = 0;
      timeline.erase(std::remove_if(std::begin(timeline),
                                    std::end(timeline),
                                    [&](std::unique_ptr<WeatherData::timelineheader> const& header) {
                                      return header->timestamp + header->expires > timeCurrent;
                                    }),
                     std::end(timeline));

      std::sort(std::begin(timeline), std::end(timeline), compareTimelineEvents);
    }

    bool WeatherService::compareTimelineEvents(const std::unique_ptr<WeatherData::timelineheader>& first,
                                               const std::unique_ptr<WeatherData::timelineheader>& second) {
      return first->timestamp > second->timestamp;
    }

    uint64_t WeatherService::getCurrentUNIXTimestamp() const {
      return std::chrono::duration_cast<std::chrono::seconds>(dateTimeController.CurrentDateTime().time_since_epoch()).count();
    }
  }
}
