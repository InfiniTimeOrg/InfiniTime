/*  Copyright (C) 2021 mruss77
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
#include "Screen.h"
#include <string>

namespace Pinetime {
  namespace Applications {
    namespace Screens {
      class FormatTime {
      public:

        struct TwelveHour {
            uint8_t Hour = 0;
            bool IsPm = false;
        };

        static TwelveHour ToTwelveHour(uint8_t hour);
        static uint8_t ToTwentyFourHour(TwelveHour hour);
        static std::string TwelveHourString(uint8_t hour, uint8_t minute);
      };
    }
  }
}