#include "components/pomodoro/SessionStatistics.h"
#include "components/datetime/DateTimeController.h"

using namespace Pinetime::Controllers;

SessionStatistics::SessionStatistics(DateTime& dateTimeController)
  : dateTimeController(dateTimeController),
    dailyCompletedSessions(0),
    dailyWorkTime(std::chrono::minutes::zero()),
    lastResetDay(GetCurrentDayOfYear()) {
}

void SessionStatistics::IncrementCompletedSessions() {
  CheckAndResetIfNewDay();
  
  if (dailyCompletedSessions < UINT8_MAX) {
    dailyCompletedSessions++;
  }
}

void SessionStatistics::AddWorkTime(std::chrono::minutes duration) {
  CheckAndResetIfNewDay();
  
  const long maxCount = UINT16_MAX;
  const long currentCount = dailyWorkTime.count();
  const long durationCount = duration.count();
  
  // Prevent overflow by checking if adding duration would exceed max
  if (currentCount + durationCount > maxCount) {
    dailyWorkTime = std::chrono::minutes(maxCount);
  } else {
    dailyWorkTime += duration;
  }
}

void SessionStatistics::ResetDailyStats() {
  dailyCompletedSessions = 0;
  dailyWorkTime = std::chrono::minutes::zero();
  lastResetDay = GetCurrentDayOfYear();
}

uint8_t SessionStatistics::GetDailyCompletedSessions() const {
  return dailyCompletedSessions;
}

std::chrono::minutes SessionStatistics::GetDailyWorkTime() const {
  return dailyWorkTime;
}

void SessionStatistics::CheckAndResetIfNewDay() {
  if (IsNewDay()) {
    ResetDailyStats();
  }
}

uint32_t SessionStatistics::GetCurrentDayOfYear() const {
  return static_cast<uint32_t>(dateTimeController.DayOfYear());
}

bool SessionStatistics::IsNewDay() const {
  uint32_t currentDay = GetCurrentDayOfYear();
  
  // Handle year rollover - if current day is significantly smaller than last reset day,
  // we've likely crossed into a new year
  if (currentDay < lastResetDay && (lastResetDay - currentDay) > 300) {
    return true;
  }
  
  return currentDay != lastResetDay;
}