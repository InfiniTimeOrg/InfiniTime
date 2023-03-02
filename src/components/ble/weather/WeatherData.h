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

/**
 * Different weather events, weather data structures used by {@link WeatherService.h}
 *
 * How to upload events to the timeline?
 *
 * All timeline write payloads are simply CBOR-encoded payloads of the structs described below.
 *
 * All payloads have a mandatory header part and the dynamic part that
 * depends on the event type specified in the header. If you don't,
 * you'll get an error returned. Data is relatively well-validated,
 * so keep in the bounds of the data types given.
 *
 * Write all struct members (CamelCase keys) into a single finite-sized map, and write it to the characteristic.
 * Mind the MTU.
 *
 * How to debug?
 *
 * There's a Screen that you can compile into your firmware that shows currently valid events.
 * You can adapt that to display something else. That part right now is very much work in progress
 * because the exact requirements are not yet known.
 *
 *
 * Implemented based on and other material:
 * https://en.wikipedia.org/wiki/METAR
 * https://www.weather.gov/jetstream/obscurationtypes
 * http://www.faraim.org/aim/aim-4-03-14-493.html
 */

namespace Pinetime {
  namespace Controllers {
    class WeatherData {
    public:
      /**
       * Visibility obscuration types
       */
      enum class obscurationtype {
        /** No obscuration */
        None = 0,
        /** Water particles suspended in the air; low visibility; does not fall */
        Fog = 1,
        /** Tiny, dry particles in the air; invisible to the eye; opalescent */
        Haze = 2,
        /** Small fire-created particles suspended in the air */
        Smoke = 3,
        /** Fine rock powder, from for example volcanoes */
        Ash = 4,
        /** Fine particles of earth suspended in the air by the wind */
        Dust = 5,
        /** Fine particles of sand suspended in the air by the wind */
        Sand = 6,
        /** Water particles suspended in the air; low-ish visibility; temperature is near dewpoint */
        Mist = 7,
        /** This is SPECIAL in the sense that the thing raining down is doing the obscuration */
        Precipitation = 8,
        Length
      };

      /**
       * Types of precipitation
       */
      enum class precipitationtype {
        /**
         * No precipitation
         *
         * Theoretically we could just _not_ send the event, but then
         * how do we differentiate between no precipitation and
         * no information about precipitation
         */
        None = 0,
        /** Drops larger than a drizzle; also widely separated drizzle */
        Rain = 1,
        /** Fairly uniform rain consisting of fine drops */
        Drizzle = 2,
        /** Rain that freezes upon contact with objects and ground */
        FreezingRain = 3,
        /** Rain + hail; ice pellets; small translucent frozen raindrops */
        Sleet = 4,
        /** Larger ice pellets; falling separately or in irregular clumps */
        Hail = 5,
        /** Hail with smaller grains of ice; mini-snowballs */
        SmallHail = 6,
        /** Snow... */
        Snow = 7,
        /** Frozen drizzle; very small snow crystals */
        SnowGrains = 8,
        /** Needles; columns or plates of ice. Sometimes described as "diamond dust". In very cold regions */
        IceCrystals = 9,
        /** It's raining down ash, e.g. from a volcano */
        Ash = 10,
        Length
      };

      /**
       * These are special events that can "enhance" the "experience" of existing weather events
       */
      enum class specialtype {
        /** Strong wind with a sudden onset that lasts at least a minute */
        Squall = 0,
        /** Series of waves in a water body caused by the displacement of a large volume of water */
        Tsunami = 1,
        /** Violent; rotating column of air */
        Tornado = 2,
        /** Unplanned; unwanted; uncontrolled fire in an area */
        Fire = 3,
        /** Thunder and/or lightning */
        Thunder = 4,
        Length
      };

      /**
       * These are used for weather timeline manipulation
       * that isn't just adding to the stack of weather events
       */
      enum class controlcodes {
        /** How much is stored already */
        GetLength = 0,
        /** This wipes the entire timeline */
        DelTimeline = 1,
        /** There's a currently valid timeline event with the given type */
        HasValidEvent = 3,
        Length
      };

      /**
       * Events have types
       * then they're easier to parse after sending them over the air
       */
      enum class eventtype : uint8_t {
        /** @see obscuration */
        Obscuration = 0,
        /** @see precipitation */
        Precipitation = 1,
        /** @see wind */
        Wind = 2,
        /** @see temperature */
        Temperature = 3,
        /** @see airquality */
        AirQuality = 4,
        /** @see special */
        Special = 5,
        /** @see pressure */
        Pressure = 6,
        /** @see location */
        Location = 7,
        /** @see cloud */
        Clouds = 8,
        /** @see humidity */
        Humidity = 9,
        Length
      };

      /**
       * Valid event query
       *
       * NOTE: Not currently available, until needs are better known
       */
      class ValidEventQuery {
      public:
        static constexpr controlcodes code = controlcodes::HasValidEvent;
        eventtype eventType;
      };

      /** The header used for further parsing */
      class TimelineHeader {
      public:
        /**
         * UNIX timestamp
         * TODO: This is currently WITH A TIMEZONE OFFSET!
         * Please send events with the timestamp offset by the timezone.
         **/
        uint64_t timestamp;
        /**
         * Time in seconds until the event expires
         *
         * 32 bits ought to be enough for everyone
         *
         * If there's a newer event of the same type then it overrides this one, even if it hasn't expired
         */
        uint32_t expires;
        /**
         * What type of weather-related event
         */
        eventtype eventType;
      };

      /** Specifies how cloudiness is stored */
      class Clouds : public TimelineHeader {
      public:
        /** Cloud coverage in percentage, 0-100% */
        uint8_t amount;
      };

      /** Specifies how obscuration is stored */
      class Obscuration : public TimelineHeader {
      public:
        /** Type of precipitation */
        obscurationtype type;
        /**
         * Visibility distance in meters
         * 65535 is reserved for unspecified
         */
        uint16_t amount;
      };

      /** Specifies how precipitation is stored */
      class Precipitation : public TimelineHeader {
      public:
        /** Type of precipitation */
        precipitationtype type;
        /**
         * How much is it going to rain? In millimeters
         * 255 is reserved for unspecified
         **/
        uint8_t amount;
      };

      /**
       * How wind speed is stored
       *
       * In order to represent bursts of wind instead of constant wind,
       * you have minimum and maximum speeds.
       *
       * As direction can fluctuate wildly and some watch faces might wish to display it nicely,
       * we're following the aerospace industry weather report option of specifying a range.
       */
      class Wind : public TimelineHeader {
      public:
        /** Meters per second */
        uint8_t speedMin;
        /** Meters per second */
        uint8_t speedMax;
        /** Unitless direction between 0-255; approximately 1 unit per 0.71 degrees */
        uint8_t directionMin;
        /** Unitless direction between 0-255; approximately 1 unit per 0.71 degrees */
        uint8_t directionMax;
      };

      /**
       * How temperature is stored
       *
       * As it's annoying to figure out the dewpoint on the watch,
       * please send it from the companion
       *
       * We don't do floats, picodegrees are not useful. Make sure to multiply.
       */
      class Temperature : public TimelineHeader {
      public:
        /**
         * Temperature °C but multiplied by 100 (e.g. -12.50°C becomes -1250)
         * -32768 is reserved for "no data"
         */
        int16_t temperature;
        /**
         * Dewpoint °C but multiplied by 100 (e.g. -12.50°C becomes -1250)
         * -32768 is reserved for "no data"
         */
        int16_t dewPoint;
      };

      /**
       * How location info is stored
       *
       * This can be mostly static with long expiration,
       * as it usually is, but it could change during a trip for ex.
       * so we allow changing it dynamically.
       *
       * Location info can be for some kind of map watch face
       * or daylight calculations, should those be required.
       *
       */
      class Location : public TimelineHeader {
      public:
        /** Location name */
        std::string location;
        /** Altitude relative to sea level in meters */
        int16_t altitude;
        /** Latitude, EPSG:3857 (Google Maps, Openstreetmaps datum) */
        int32_t latitude;
        /** Longitude, EPSG:3857 (Google Maps, Openstreetmaps datum) */
        int32_t longitude;
      };

      /**
       * How humidity is stored
       */
      class Humidity : public TimelineHeader {
      public:
        /** Relative humidity, 0-100% */
        uint8_t humidity;
      };

      /**
       * How air pressure is stored
       */
      class Pressure : public TimelineHeader {
      public:
        /** Air pressure in hectopascals (hPa) */
        int16_t pressure;
      };

      /**
       * How special events are stored
       */
      class Special : public TimelineHeader {
      public:
        /** Special event's type */
        specialtype type;
      };

      /**
       * How air quality is stored
       *
       * These events are a bit more complex because the topic is not simple,
       * the intention is to heavy-lift the annoying preprocessing from the watch
       * this allows watch face or watchapp makers to generate accurate alerts and graphics
       *
       * If this needs further enforced standardization, pull requests are welcome
       */
      class AirQuality : public TimelineHeader {
      public:
        /**
         * The name of the pollution
         *
         * for the sake of better compatibility with watchapps
         * that might want to use this data for say visuals
         * don't localize the name.
         *
         * Ideally watchapp itself localizes the name, if it's at all needed.
         *
         * E.g.
         * For generic ones use "PM0.1", "PM5", "PM10"
         * For chemical compounds use the molecular formula e.g. "NO2", "CO2", "O3"
         * For pollen use the genus, e.g. "Betula" for birch or "Alternaria" for that mold's spores
         */
        std::string polluter;
        /**
         * Amount of the pollution in SI units,
         * otherwise it's going to be difficult to create UI, alerts
         * and so on and for.
         *
         * See more:
         * https://ec.europa.eu/environment/air/quality/standards.htm
         * http://www.ourair.org/wp-content/uploads/2012-aaqs2.pdf
         *
         * Example units:
         * count/m³ for pollen
         * µgC/m³ for micrograms of organic carbon
         * µg/m³ sulfates, PM0.1, PM1, PM2, PM10 and so on, dust
         * mg/m³ CO2, CO
         * ng/m³ for heavy metals
         *
         * List is not comprehensive, should be improved.
         * The current ones are what watchapps assume!
         *
         * Note: ppb and ppm to concentration should be calculated on the companion, using
         * the correct formula (taking into account temperature and air pressure)
         *
         * Note2: The amount is off by times 100, for two decimal places of precision.
         * E.g. 54.32µg/m³ is 5432
         *
         */
        uint32_t amount;
      };
    };
  }
}
