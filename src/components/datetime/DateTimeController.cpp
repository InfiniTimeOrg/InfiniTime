#include "components/datetime/DateTimeController.h"
#include <libraries/log/nrf_log.h>
#include <systemtask/SystemTask.h>
#include <hal/nrf_rtc.h>
#include "nrf_assert.h"

using namespace Pinetime::Controllers;

namespace {
  constexpr const char* const DaysStringShort[] = {"--", "MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN"};
  constexpr const char* const DaysStringShortLow[] = {"--", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
  constexpr const char* const DaysString[] = {"--", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY", "SUNDAY"};
  constexpr const char* const DaysStringLow[] = {"--", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
  constexpr const char* const MonthsString[] = {"--", "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
  constexpr const char* const MonthsStringLow[] =
    {"--", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

  constexpr int compileTimeAtoi(const char* str) {
    int result = 0;
    while (*str >= '0' && *str <= '9') {
      result = result * 10 + *str - '0';
      str++;
    }
    return result;
  }
}

DateTime::DateTime(Controllers::Settings& settingsController) : settingsController {settingsController} {
  mutex = xSemaphoreCreateMutex();
  ASSERT(mutex != nullptr);
  xSemaphoreGive(mutex);

  // __DATE__ is a string of the format "MMM DD YYYY", so an offset of 7 gives the start of the year
  SetTime(compileTimeAtoi(&__DATE__[7]), 1, 1, 0, 0, 0);
}

void DateTime::SetCurrentTime(std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> t) {
  xSemaphoreTake(mutex, portMAX_DELAY);
  this->currentDateTime = t;
  UpdateTime(previousSystickCounter, true); // Update internal state without updating the time
  xSemaphoreGive(mutex);
}

void DateTime::SetTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
  std::tm tm = {
    /* .tm_sec  = */ second,
    /* .tm_min  = */ minute,
    /* .tm_hour = */ hour,
    /* .tm_mday = */ day,
    /* .tm_mon  = */ month - 1,
    /* .tm_year = */ year - 1900,
  };

  NRF_LOG_INFO("%d %d %d ", day, month, year);
  NRF_LOG_INFO("%d %d %d ", hour, minute, second);

  tm.tm_isdst = -1; // Use DST value from local time zone

  xSemaphoreTake(mutex, portMAX_DELAY);
  currentDateTime = std::chrono::system_clock::from_time_t(std::mktime(&tm));
  UpdateTime(previousSystickCounter, true);
  xSemaphoreGive(mutex);

  if (systemTask != nullptr) {
    systemTask->PushMessage(System::Messages::OnNewTime);
  }
}

void DateTime::SetTimeZone(int8_t timezone, int8_t dst) {
  tzOffset = timezone;
  dstOffset = dst;
}

std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> DateTime::CurrentDateTime() {
  xSemaphoreTake(mutex, portMAX_DELAY);
  UpdateTime(nrf_rtc_counter_get(portNRF_RTC_REG), false);
  xSemaphoreGive(mutex);
  return currentDateTime;
}

void DateTime::UpdateTime(uint32_t systickCounter, bool forceUpdate) {
  // Handle systick counter overflow
  uint32_t systickDelta = 0;
  if (systickCounter < previousSystickCounter) {
    systickDelta = static_cast<uint32_t>(portNRF_RTC_MAXTICKS) - previousSystickCounter;
    systickDelta += systickCounter + 1;
  } else {
    systickDelta = systickCounter - previousSystickCounter;
  }

  auto correctedDelta = systickDelta / configTICK_RATE_HZ;
  // If a second hasn't passed, there is nothing to do
  // If the time has been changed, set forceUpdate to trigger internal state updates
  if (correctedDelta == 0 && !forceUpdate) {
    return;
  }
  auto rest = systickDelta % configTICK_RATE_HZ;
  if (systickCounter >= rest) {
    previousSystickCounter = systickCounter - rest;
  } else {
    previousSystickCounter = static_cast<uint32_t>(portNRF_RTC_MAXTICKS) - (rest - systickCounter - 1);
  }

  currentDateTime += std::chrono::seconds(correctedDelta);
  uptime += std::chrono::seconds(correctedDelta);

  std::time_t currentTime = std::chrono::system_clock::to_time_t(currentDateTime);
  localTime = *std::localtime(&currentTime);

  auto minute = Minutes();
  auto hour = Hours();

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
  return MonthsString[static_cast<uint8_t>(Month())];
}

const char* DateTime::DayOfWeekShortToString() const {
  return DaysStringShort[static_cast<uint8_t>(DayOfWeek())];
}

const char* DateTime::DayOfWeekToString() const {
  return DaysString[static_cast<uint8_t>(DayOfWeek())];
}

const char* DateTime::MonthShortToStringLow(Months month) {
  return MonthsStringLow[static_cast<uint8_t>(month)];
}

const char* DateTime::DayOfWeekShortToStringLow(Days day) {
  return DaysStringShortLow[static_cast<uint8_t>(day)];
}

const char* DateTime::DayOfWeekToStringLow(Days day) {
  return DaysStringLow[static_cast<uint8_t>(day)];
}

void DateTime::Register(Pinetime::System::SystemTask* systemTask) {
  this->systemTask = systemTask;
}

using ClockType = Pinetime::Controllers::Settings::ClockType;

std::string DateTime::FormattedTime() {
  auto hour = Hours();
  auto minute = Minutes();
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
    snprintf(buff, sizeof(buff), "%i:%02i %s", hour12, minute, amPmStr);
  } else {
    snprintf(buff, sizeof(buff), "%02i:%02i", hour, minute);
  }
  return std::string(buff);
}
