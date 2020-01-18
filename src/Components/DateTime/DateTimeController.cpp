#include "DateTimeController.h"
#include <date/date.h>
#include <libraries/log/nrf_log.h>

using namespace Pinetime::Controllers;


void DateTime::SetTime(uint16_t year, uint8_t month, uint8_t day, uint8_t dayOfWeek, uint8_t hour, uint8_t minute,
                       uint8_t second, uint32_t systickCounter) {

  currentDateTime = {};
  currentDateTime += date::years( year-1970);
  currentDateTime += date::days( day - 1);
  currentDateTime += date::months( month - 1);

  currentDateTime += std::chrono::hours(hour);
  currentDateTime += std::chrono::minutes (minute);
  currentDateTime += std::chrono::seconds (second);

  currentDateTime -= std::chrono::hours(3); // TODO WHYYYY?
  NRF_LOG_INFO("%d %d %d ", day, month, year);
  NRF_LOG_INFO("%d %d %d ", hour, minute, second);
  previousSystickCounter = systickCounter;
  UpdateTime(systickCounter);
  NRF_LOG_INFO("* %d %d %d ", this->hour, this->minute, this->second);
  NRF_LOG_INFO("* %d %d %d ", this->day, this->month, this->year);


}

void DateTime::UpdateTime(uint32_t systickCounter) {
  uint32_t systickDelta = 0;
  if(systickCounter < previousSystickCounter) {
    systickDelta = 0xffffff - previousSystickCounter;
    systickDelta += systickCounter + 1;
  } else {
    systickDelta = systickCounter - previousSystickCounter;
  }

  previousSystickCounter = systickCounter;
  currentDateTime += std::chrono::milliseconds(systickDelta);

  auto dp = date::floor<date::days>(currentDateTime);
  auto time = date::make_time(currentDateTime-dp);
  auto yearMonthDay = date::year_month_day(dp);

  year = (int)yearMonthDay.year();
  month = static_cast<Months>((unsigned)yearMonthDay.month());
  day = (unsigned)yearMonthDay.day();
  dayOfWeek = static_cast<Days>(date::weekday(yearMonthDay).iso_encoding());

  hour = time.hours().count();
  minute = time.minutes().count();
  second = time.seconds().count();
}

