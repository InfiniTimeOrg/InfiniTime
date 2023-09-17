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
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#define min // workaround: nimble's min/max macros conflict with libstdc++
#define max
#include <host/ble_gap.h>
#include <host/ble_uuid.h>
#undef max
#undef min

#include "WeatherData.h"
#include "libs/QCBOR/inc/qcbor/qcbor.h"
#include "components/datetime/DateTimeController.h"

int WeatherCallback(uint16_t connHandle, uint16_t attrHandle, struct ble_gatt_access_ctxt* ctxt, void* arg);

namespace Pinetime {
  namespace Controllers {

    class WeatherService {
    public:
      explicit WeatherService(const DateTime& dateTimeController);

      void Init();

      int OnCommand(struct ble_gatt_access_ctxt* ctxt);

      /*
       * Helper functions for quick access to currently valid data
       */
      std::unique_ptr<WeatherData::Location>& GetCurrentLocation();
      std::unique_ptr<WeatherData::Clouds>& GetCurrentClouds();
      std::unique_ptr<WeatherData::Obscuration>& GetCurrentObscuration();
      std::unique_ptr<WeatherData::Precipitation>& GetCurrentPrecipitation();
      std::unique_ptr<WeatherData::Wind>& GetCurrentWind();
      std::unique_ptr<WeatherData::Temperature>& GetCurrentTemperature();
      std::unique_ptr<WeatherData::Humidity>& GetCurrentHumidity();
      std::unique_ptr<WeatherData::Pressure>& GetCurrentPressure();
      std::unique_ptr<WeatherData::AirQuality>& GetCurrentQuality();

      /**
       * Searches for the current day's maximum temperature
       * @return -32768 if there's no data, degrees Celsius times 100 otherwise
       */
      int16_t GetTodayMaxTemp() const;
      /**
       * Searches for the current day's minimum temperature
       * @return -32768 if there's no data, degrees Celsius times 100 otherwise
       */
      int16_t GetTodayMinTemp() const;

      /*
       * Management functions
       */
      /**
       * Adds an event to the timeline
       * @return
       */
      bool AddEventToTimeline(std::unique_ptr<WeatherData::TimelineHeader> event);
      /**
       * Gets the current timeline length
       */
      size_t GetTimelineLength() const;
      /**
       * Checks if an event of a certain type exists in the timeline
       */
      bool HasTimelineEventOfType(WeatherData::eventtype type) const;

    private:
      // 00040000-78fc-48fe-8e23-433b3a1942d0
      static constexpr ble_uuid128_t BaseUuid() {
        return CharUuid(0x00, 0x00);
      }

      // 0004yyxx-78fc-48fe-8e23-433b3a1942d0
      static constexpr ble_uuid128_t CharUuid(uint8_t x, uint8_t y) {
        return ble_uuid128_t {.u = {.type = BLE_UUID_TYPE_128},
                              .value = {0xd0, 0x42, 0x19, 0x3a, 0x3b, 0x43, 0x23, 0x8e, 0xfe, 0x48, 0xfc, 0x78, y, x, 0x04, 0x00}};
      }

      static constexpr size_t maxNbElements = 10;

      ble_uuid128_t weatherUuid {BaseUuid()};

      /**
       * Just write timeline data here.
       *
       * See {@link WeatherData.h} for more information.
       */
      ble_uuid128_t weatherDataCharUuid {CharUuid(0x00, 0x01)};
      /**
       * This doesn't take timeline data, provides some control over it.
       *
       * NOTE: Currently not supported. Companion app implementer feedback required.
       * There's very little point in solidifying an API before we know the needs.
       */
      ble_uuid128_t weatherControlCharUuid {CharUuid(0x00, 0x02)};

      const struct ble_gatt_chr_def characteristicDefinition[3] = {
        {.uuid = &weatherDataCharUuid.u,
         .access_cb = WeatherCallback,
         .arg = this,
         .flags = BLE_GATT_CHR_F_WRITE,
         .val_handle = &eventHandle},
        {.uuid = &weatherControlCharUuid.u, .access_cb = WeatherCallback, .arg = this, .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ},
        {nullptr}};
      const struct ble_gatt_svc_def serviceDefinition[2] = {
        {.type = BLE_GATT_SVC_TYPE_PRIMARY, .uuid = &weatherUuid.u, .characteristics = characteristicDefinition},
        {0}};

      uint16_t eventHandle {};

      const Pinetime::Controllers::DateTime& dateTimeController;

      std::vector<std::unique_ptr<WeatherData::TimelineHeader>> timeline;
      std::unique_ptr<WeatherData::TimelineHeader> nullTimelineheader = std::make_unique<WeatherData::TimelineHeader>();
      std::unique_ptr<WeatherData::TimelineHeader>* nullHeader;

      /**
       * Cleans up the timeline of expired events
       */
      void TidyTimeline();

      /**
       * Compares two timeline events
       */
      static bool CompareTimelineEvents(const std::unique_ptr<WeatherData::TimelineHeader>& first,
                                        const std::unique_ptr<WeatherData::TimelineHeader>& second);

      /**
       * Returns current UNIX timestamp
       */
      uint64_t GetCurrentUnixTimestamp() const;

      /**
       * Checks if the event hasn't gone past and expired
       *
       * @param header timeline event to check
       * @param currentTimestamp what's the time right now
       * @return if the event is valid
       */
      static bool IsEventStillValid(const std::unique_ptr<WeatherData::TimelineHeader>& uniquePtr, const uint64_t timestamp);

      /**
       * This is a helper function that closes a QCBOR map and decoding context cleanly
       */
      void CleanUpQcbor(QCBORDecodeContext* decodeContext);
    };
  }
}
