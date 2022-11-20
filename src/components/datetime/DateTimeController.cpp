#include "components/datetime/DateTimeController.h"
#include <date/date.h>
#include <libraries/log/nrf_log.h>
#include <systemtask/SystemTask.h>

using namespace Pinetime::Controllers;

namespace {
  char const* DaysStringShort[] = {"--", "MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};
  char const* DaysStringShortLow[] = {"--", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
  char const* MonthsString[] = {"--", "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
  char const* MonthsStringLow[] = {"--", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
}

DateTime::DateTime(Controllers::Settings& settingsController) : settingsController {settingsController} {
}

void DateTime::SetCurrentTime(std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> t) {
  this->currentDateTime = t;
  UpdateTime(previousSystickCounter); // Update internal state without updating the time
}

void DateTime::SetTime(uint16_t year,
                       uint8_t month,
                       uint8_t day,
                       uint8_t dayOfWeek,
                       uint8_t hour,
                       uint8_t minute,
                       uint8_t second,
                       uint32_t systickCounter) {
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

  systemTask->PushMessage(System::Messages::OnNewTime);
}

void DateTime::SetTimeZone(uint8_t timezone, uint8_t dst) {
  tzOffset = timezone;
  dstOffset = dst;
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

  if (minute == 0 && !isHourAlreadyNotified) {
    isHourAlreadyNotified = true;
    if (systemTask != nullptr) {
      systemTask->PushMessage(System::Messages::OnNewHour);
    }
  } else if (minute != 0) {
    isHourAlreadyNotified = false;
  }

  if ((minute == 0 || minute == 30) && !isHalfHourAlreadyNotified) {
    isHalfHourAlreadyNotified = true;
    if (systemTask != nullptr) {
      systemTask->PushMessage(System::Messages::OnNewHalfHour);
    }
  } else if (minute != 0 && minute != 30) {
    isHalfHourAlreadyNotified = false;
  }

  // Notify new day to SystemTask
  if (hour == 0 and not isMidnightAlreadyNotified) {
    isMidnightAlreadyNotified = true;
    if (systemTask != nullptr)
      systemTask->PushMessage(System::Messages::OnNewDay);
  } else if (hour != 0) {
    isMidnightAlreadyNotified = false;
  }
}

const char* DateTime::MonthShortToString() const {
  return MonthsString[static_cast<uint8_t>(month)];
}

const char* DateTime::DayOfWeekShortToString() const {
  return DaysStringShort[static_cast<uint8_t>(dayOfWeek)];
}

const char* DateTime::MonthShortToStringLow(Months month) {
  return MonthsStringLow[static_cast<uint8_t>(month)];
}

const char* DateTime::DayOfWeekShortToStringLow() const {
  return DaysStringShortLow[static_cast<uint8_t>(dayOfWeek)];
}

void DateTime::Register(Pinetime::System::SystemTask* systemTask) {
  this->systemTask = systemTask;
}

using ClockType = Pinetime::Controllers::Settings::ClockType;
std::string DateTime::FormattedTime() {
  // Return time as a string in 12- or 24-hour format
  char buff[9];
  if (settingsController.GetClockType() == ClockType::H12) {
    uint8_t hour12;
    const char* amPmStr;
    if (hour < 12) {
      hour12 = (hour == 0) ? 12 : hour;
      amPmStr = "AM";
    } else {
      hour12 = (hour == 12) ? 12 : hour - 12;
      amPmStr = "PM";
    }
    sprintf(buff, "%i:%02i %s", hour12, minute, amPmStr);
  } else {
    sprintf(buff, "%02i:%02i", hour, minute);
  }
  return std::string(buff);
}
