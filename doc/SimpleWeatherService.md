# Simple Weather Service

## Introduction

The Simple Weather Service provides a simple and straightforward API to specify the current weather and the forecast for the next 5 days.
It effectively replaces the original Weather Service (from InfiniTime 1.8) since InfiniTime 1.14.

## Service

The service UUID is `00050000-78fc-48fe-8e23-433b3a1942d0`.

## Characteristics

## Weather data (UUID 00050001-78fc-48fe-8e23-433b3a1942d0)

The host uses this characteristic to update the current weather information and the forecast for the next 5 days.

This characteristics accepts a byte array with the following 2-Bytes header:

 - [0] Message Type : 
   - `0` : Current weather
   - `1` : Forecast
 - [1] Message Version : Version `0` is currently supported. Other versions might be added in future releases

### Current Weather 

The byte array must contain the following data:

 - [0] : Message type = `0`
 - [1] : Message version = `0`
 - [2][3][4][5][6][7][8][9] : Timestamp (64 bits UNIX timestamp, number of nanoseconds elapsed since 1 JAN 1970)
 - [10, 11] : Current temperature (°C * 100)
 - [12, 13] : Minimum temperature (°C * 100)
 - [14, 15] : Maximum temperature (°C * 100)
 - [16]..[47] : location (string, unused characters should be set to `0`)
 - [48] : icon ID 
   - 0 = Sun, clear sky
   - 1 = Few clouds
   - 2 = Clouds
   - 3 = Heavy clouds
   - 4 = Clouds & rain
   - 5 = Rain
   - 6 = Thunderstorm
   - 7 = Snow
   - 8 = Mist, smog

### Forecast

The byte array must contain the following data:

  - [0] : Message type = `0`
  - [1] : Message version = `0`
  - [2][3][4][5][6][7][8][9] : Timestamp (64 bits UNIX timestamp, number of nanoseconds elapsed since 1 JAN 1970)
  - [10] Number of days (Max 5, fields for unused days should be set to `0`)
  - [11,12] Day 0 Minimum temperature (°C * 100)
  - [13,14] Day 0 Maximum temperature (°C * 100)
  - [15] Day 0 Icon ID
  - [16,17] Day 1 Minimum temperature (°C * 100)
  - [18,19] Day 1 Maximum temperature (°C * 100)
  - [20] Day 1 Icon ID
  - [21,22] Day 2 Minimum temperature (°C * 100)
  - [23,24] Day 2 Maximum temperature (°C * 100)
  - [25] Day 2 Icon ID
  - [26,27] Day 3 Minimum temperature (°C * 100)
  - [28,29] Day 3 Maximum temperature (°C * 100)
  - [30] Day 3 Icon ID
  - [31,32] Day 4 Minimum temperature (°C * 100)
  - [33,34] Day 4 Maximum temperature (°C * 100)
  - [35] Day 4 Icon ID