#include <cstdio>
#include <libs/date/includes/date/date.h>
#include <Components/DateTime/DateTimeController.h>
#include <Version.h>
#include "Clock.h"

using namespace Pinetime::Applications::Screens;

void Clock::Refresh(bool fullRefresh) {
  if(fullRefresh) {
    gfx.FillRectangle(0,0,240,240,0x0000);
    currentChar[0] = 0;
    currentChar[1] = 0;
    currentChar[2] = 0;
    currentChar[3] = 0;
    auto dummy = currentDateTime.Get();
  }

  if (fullRefresh || batteryPercentRemaining.IsUpdated()) {
    char batteryChar[11];
    auto newBatteryValue = batteryPercentRemaining.Get();
    newBatteryValue = (newBatteryValue > 100) ? 100 : newBatteryValue;
    newBatteryValue = (newBatteryValue < 0) ? 0 : newBatteryValue;

    sprintf(batteryChar, "BAT: %d%%", newBatteryValue);
    gfx.DrawString((240 - 108), 0, 0xffff, batteryChar, &smallFont, false);
  }

  if (fullRefresh || bleState.IsUpdated()) {
    uint16_t color = (bleState.Get() == BleConnectionStates::Connected) ? 0xffff : 0x0000;
    gfx.DrawString(10, 0, color, "BLE", &smallFont, false);
  }

  if(fullRefresh || currentDateTime.IsUpdated()) {
    auto newDateTime = currentDateTime.Get();

    auto dp = date::floor<date::days>(newDateTime);
    auto time = date::make_time(newDateTime-dp);
    auto yearMonthDay = date::year_month_day(dp);

    auto year = (int)yearMonthDay.year();
    auto month = static_cast<Pinetime::Controllers::DateTime::Months>((unsigned)yearMonthDay.month());
    auto day = (unsigned)yearMonthDay.day();
    auto dayOfWeek = static_cast<Pinetime::Controllers::DateTime::Days>(date::weekday(yearMonthDay).iso_encoding());

    auto hour = time.hours().count();
    auto minute = time.minutes().count();
    auto second = time.seconds().count();

    char minutesChar[3];
    sprintf(minutesChar, "%02d", minute);

    char hoursChar[3];
    sprintf(hoursChar, "%02d", hour);

    uint8_t x = 7;
    if (hoursChar[0] != currentChar[0]) {
      gfx.DrawChar(&largeFont, hoursChar[0], &x, 78, 0xffff);
      currentChar[0] = hoursChar[0];
    }

    x = 61;
    if (hoursChar[1] != currentChar[1]) {
      gfx.DrawChar(&largeFont, hoursChar[1], &x, 78, 0xffff);
      currentChar[1] = hoursChar[1];
    }

    x = 127;
    if (minutesChar[0] != currentChar[2]) {
      gfx.DrawChar(&largeFont, minutesChar[0], &x, 78, 0xffff);
      currentChar[2] = minutesChar[0];
    }

    x = 181;
    if (minutesChar[1] != currentChar[3]) {
      gfx.DrawChar(&largeFont, minutesChar[1], &x, 78, 0xffff);
      currentChar[3] = minutesChar[1];
    }

    if ((year != currentYear) || (month != currentMonth) || (dayOfWeek != currentDayOfWeek) || (day != currentDay)) {
      gfx.FillRectangle(0,180, 240, 15, 0x0000);
      char dateStr[22];
      sprintf(dateStr, "%s %d %s %d", DayOfWeekToString(dayOfWeek), day, MonthToString(month), year);
      gfx.DrawString(10, 180, 0xffff, dateStr, &smallFont, false);

      currentYear = year;
      currentMonth = month;
      currentDayOfWeek = dayOfWeek;
      currentDay = day;
    }
  }

  if(fullRefresh || version.IsUpdated()) {
    char version[20];
    sprintf(version, "VERSION: %d.%d.%d", Version::Major(), Version::Minor(), Version::Patch());
    gfx.DrawString(20, 220, 0xffff, version, &smallFont, false);
  }
}

const char *Clock::MonthToString(Pinetime::Controllers::DateTime::Months month) {
  return Clock::MonthsString[static_cast<uint8_t>(month)];
}

const char *Clock::DayOfWeekToString(Pinetime::Controllers::DateTime::Days dayOfWeek) {
  return Clock::DaysString[static_cast<uint8_t>(dayOfWeek)];
}

char const *Clock::DaysString[] = {
        "",
        "MONDAY",
        "TUESDAY",
        "WEDNESDAY",
        "THURSDAY",
        "FRIDAY",
        "SATURDAY",
        "SUNDAY"
};

char const *Clock::MonthsString[] = {
        "",
        "JAN",
        "FEB",
        "MAR",
        "APR",
        "MAY",
        "JUN",
        "JUL",
        "AUG",
        "SEP",
        "OCT",
        "NOV",
        "DEC"
};
