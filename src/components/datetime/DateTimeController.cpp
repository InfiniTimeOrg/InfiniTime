#include "DateTimeController.h"
#include <date/date.h>
#include <libraries/log/nrf_log.h>
#include <systemtask/SystemTask.h>

using namespace Pinetime::Controllers;

void DateTime::SetTime(
  uint16_t year, uint8_t month, uint8_t day, uint8_t dayOfWeek, uint8_t hour, uint8_t minute, uint8_t second, uint32_t systickCounter) {
  std::tm tm = {
    /* .tm_sec  = */ second,
    /* .tm_min  = */ minute,
    /* .tm_hour = */ hour,
    /* .tm_mday = */ day,
    /* .tm_mon  = */ month - 1,
    /* .tm_year = */ year - 1900,
  };
  tm.tm_isdst = -1; // Use DST value from local time zone
  currentDateTime = std::chrono::system_clock::from_time_t(std::mktime(&tm));

  NRF_LOG_INFO("%d %d %d ", day, month, year);
  NRF_LOG_INFO("%d %d %d ", hour, minute, second);
  previousSystickCounter = systickCounter;

  UpdateTime(systickCounter);
  NRF_LOG_INFO("* %d %d %d ", this->hour, this->minute, this->second);
  NRF_LOG_INFO("* %d %d %d ", this->day, this->month, this->year);
}

void DateTime::UpdateTime(uint32_t systickCounter) {
  // Handle systick counter overflow
  uint32_t systickDelta = 0;
  if (systickCounter < previousSystickCounter) {
    systickDelta = 0xffffff - previousSystickCounter;
    systickDelta += systickCounter + 1;
  } else {
    systickDelta = systickCounter - previousSystickCounter;
  }

  /*
   * 1000 ms = 1024 ticks
   */
  auto correctedDelta = systickDelta / 1024;
  auto rest = (systickDelta - (correctedDelta * 1024));
  if (systickCounter >= rest) {
    previousSystickCounter = systickCounter - rest;
  } else {
    previousSystickCounter = 0xffffff - (rest - systickCounter);
  }

  currentDateTime += std::chrono::seconds(correctedDelta);
  uptime += std::chrono::seconds(correctedDelta);

  auto dp = date::floor<date::days>(currentDateTime);
  auto time = date::make_time(currentDateTime - dp);
  auto yearMonthDay = date::year_month_day(dp);

  year = static_cast<int>(yearMonthDay.year());
  month = static_cast<Months>(static_cast<unsigned>(yearMonthDay.month()));
  day = static_cast<unsigned>(yearMonthDay.day());
  dayOfWeek = static_cast<Days>(date::weekday(yearMonthDay).iso_encoding());

  hour = time.hours().count();
  minute = time.minutes().count();
  second = time.seconds().count();

  // Notify new day to SystemTask
  if (hour == 0 and not isMidnightAlreadyNotified) {
    isMidnightAlreadyNotified = true;
    if(systemTask != nullptr)
      systemTask->PushMessage(System::Messages::OnNewDay);
  } else if (hour != 0) {
    isMidnightAlreadyNotified = false;
  }
}

const char* DateTime::MonthShortToString() {
  return DateTime::MonthsString[static_cast<uint8_t>(month)];
}

const char* DateTime::MonthShortToStringLow() {
  return DateTime::MonthsStringLow[static_cast<uint8_t>(month)];
}

const char* DateTime::MonthsToStringLow() {
  return DateTime::MonthsLow[static_cast<uint8_t>(month)];
}

const char* DateTime::DayOfWeekToString() {
  return DateTime::DaysString[static_cast<uint8_t>(dayOfWeek)];
}

const char* DateTime::DayOfWeekShortToString() {
  return DateTime::DaysStringShort[static_cast<uint8_t>(dayOfWeek)];
}

const char* DateTime::DayOfWeekToStringLow() {
  return DateTime::DaysStringLow[static_cast<uint8_t>(dayOfWeek)];
}

const char* DateTime::DayOfWeekShortToStringLow() {
  return DateTime::DaysStringShortLow[static_cast<uint8_t>(dayOfWeek)];
}

void DateTime::Register(Pinetime::System::SystemTask* systemTask) {
  this->systemTask = systemTask;
}

char const* DateTime::DaysStringLow[] = {"--", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};

char const* DateTime::DaysStringShortLow[] = {"--", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

char const* DateTime::DaysStringShort[] = {"--", "MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};

char const* DateTime::DaysString[] = {"--", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY", "SUNDAY"};

char const* DateTime::MonthsString[] = {"--", "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

char const* DateTime::MonthsStringLow[] = {"--", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

char const* DateTime::MonthsLow[] = {
  "--", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};