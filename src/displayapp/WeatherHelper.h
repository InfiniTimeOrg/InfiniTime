/*  Copyright (C) 2024 Caleb Fontenot

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

#include <FreeRTOS.h>

namespace Pinetime {
  namespace Applications {
      class WeatherHelper {
        public:
            static int16_t RoundTemperature(int16_t temp);
            static const lv_color_t TemperatureColor(int16_t temperature);
            static const char* floatToRgbHex(lv_color_t rgb);
      };
    }
 }
