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
#include <components/datetime/DateTimeController.h>

// 00030000-78fc-48fe-8e23-433b3a1942d0
#define WEATHER_SERVICE_UUID_BASE                                                                                                          \
  { 0xd0, 0x42, 0x19, 0x3a, 0x3b, 0x43, 0x23, 0x8e, 0xfe, 0x48, 0xfc, 0x78, 0x00, 0x00, 0x03, 0x00 }
#define WEATHER_SERVICE_CHAR_UUID(y, x)                                                                                                    \
  { 0xd0, 0x42, 0x19, 0x3a, 0x3b, 0x43, 0x23, 0x8e, 0xfe, 0x48, 0xfc, 0x78, (x), (y), 0x03, 0x00 }

int WeatherCallback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt, void* arg);

namespace Pinetime {
  namespace System {
    class SystemTask;
  }
  namespace Controllers {

    class WeatherService {
    public:
      explicit WeatherService(System::SystemTask& system, DateTime& dateTimeController);

      void Init();

      int OnCommand(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt* ctxt);

      /*
       * Helper functions for quick access to currently valid data
       */
      WeatherData::Location getCurrentLocation() const;
      WeatherData::Clouds getCurrentClouds() const;
      WeatherData::Obscuration getCurrentObscuration() const;
      WeatherData::Precipitation getCurrentPrecipitation() const;
      WeatherData::Wind getCurrentWind() const;
      WeatherData::Temperature getCurrentTemperature() const;
      WeatherData::Humidity getCurrentHumidity() const;
      WeatherData::Pressure getCurrentPressure() const;
      WeatherData::AirQuality getCurrentQuality() const;

      /*
       * Management functions
       */
      /**
       * Adds an event to the timeline
       * @return
       */
      bool addEventToTimeline(std::unique_ptr<WeatherData::TimelineHeader> event);
      /**
       * Gets the current timeline length
       */
      size_t getTimelineLength() const;
      /**
       * Checks if an event of a certain type exists in the timeline
       * @return
       */
      bool hasTimelineEventOfType(WeatherData::eventtype type) const;

    private:
      ble_uuid128_t weatherUUID {.u = {.type = BLE_UUID_TYPE_128}, .value = WEATHER_SERVICE_UUID_BASE};

      /**
       * Just write timeline data here
       */
      ble_uuid128_t weatherDataCharUUID {.u = {.type = BLE_UUID_TYPE_128}, .value = WEATHER_SERVICE_CHAR_UUID(0x00, 0x01)};
      /**
       * This doesn't take timeline data,
       * provides some control over it
       */
      ble_uuid128_t weatherControlCharUUID {.u = {.type = BLE_UUID_TYPE_128}, .value = WEATHER_SERVICE_CHAR_UUID(0x00, 0x02)};

      const struct ble_gatt_chr_def characteristicDefinition[2] = {{.uuid = &weatherDataCharUUID.u,
                                                                    .access_cb = WeatherCallback,
                                                                    .arg = this,
                                                                    .flags = BLE_GATT_CHR_F_NOTIFY,
                                                                    .val_handle = &eventHandle},
                                                                   {.uuid = &weatherControlCharUUID.u,
                                                                    .access_cb = WeatherCallback,
                                                                    .arg = this,
                                                                    .flags = BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_F_READ}};
      const struct ble_gatt_svc_def serviceDefinition[2] = {
        {.type = BLE_GATT_SVC_TYPE_PRIMARY, .uuid = &weatherUUID.u, .characteristics = characteristicDefinition}, {0}};

      uint16_t eventHandle {};

      Pinetime::System::SystemTask& system;
      Pinetime::Controllers::DateTime& dateTimeController;

      std::vector<std::unique_ptr<WeatherData::TimelineHeader>> timeline;

      /**
       * Cleans up the timeline of expired events
       * @return result code
       */
      void tidyTimeline();

      /**
       * Compares two timeline events
       */
      static bool compareTimelineEvents(const std::unique_ptr<WeatherData::TimelineHeader>& first,
                                        const std::unique_ptr<WeatherData::TimelineHeader>& second);

      /**
       * Returns current UNIX timestamp
       */
      uint64_t getCurrentUNIXTimestamp() const;
    };
  }
}
