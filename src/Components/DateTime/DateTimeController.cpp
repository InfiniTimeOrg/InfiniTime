#include "DateTimeController.h"

using namespace Pinetime::Controllers;


void DateTime::UpdateTime(uint16_t year, Months month, uint8_t day, Days dayOfWeek, uint8_t hour, uint8_t minute,
                          uint8_t second) {
  this->year = year;
  this->month = month;
  this->dayOfWeek = dayOfWeek;
  this->day = day;
  this->hour = hour;
  this->minute = minute;
  this->second = second;
}

