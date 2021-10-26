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
#include "FormatTime.h"
#include <string>

using namespace Pinetime::Applications::Screens;

FormatTime::TwelveHour FormatTime::ToTwelveHour(uint8_t hour) {
  TwelveHour t12Hr;
  if (hour < 12) {
    t12Hr.Hour = (hour == 0) ? 12 : hour;
    t12Hr.IsPm = false;
  } else {
    t12Hr.Hour = (hour == 12) ? 12 : hour - 12;
    t12Hr.IsPm = true;
  }
  return t12Hr;
}

uint8_t FormatTime::ToTwentyFourHour(FormatTime::TwelveHour hour) {
  if (!hour.IsPm && hour.Hour == 12) {
    return 0;
  }
  uint8_t t24Hour = hour.Hour;
  if (hour.IsPm && hour.Hour < 12) {
    t24Hour += 12;
  }
  return t24Hour;
}

std::string FormatTime::TwelveHourString(uint8_t hour, uint8_t minute) {
  FormatTime::TwelveHour t12 = FormatTime::ToTwelveHour(hour);
  const char* amPmStr = t12.IsPm ? "PM" : "AM";
  char buff[9];
  sprintf(buff, "%i:%02i %s", t12.Hour, minute, amPmStr);
  return std::string(buff);
}
