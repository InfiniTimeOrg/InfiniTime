#pragma once

#include <chrono>
#include <cstdint>

namespace Pinetime {
  namespace Controllers {
    class DateTime;
    
    class SessionStatistics {
    public:
      explicit SessionStatistics(DateTime& dateTimeController);
      
      // Session tracking methods
      void IncrementCompletedSessions();
      void AddWorkTime(std::chrono::minutes duration);
      void ResetDailyStats();
      
      // Statistics retrieval methods
      uint8_t GetDailyCompletedSessions() const;
      std::chrono::minutes GetDailyWorkTime() const;
      
      // Internal maintenance
      void CheckAndResetIfNewDay();
      
    private:
      DateTime& dateTimeController;
      
      // Daily statistics
      uint8_t dailyCompletedSessions;
      std::chrono::minutes dailyWorkTime;
      
      // Day tracking for midnight reset
      uint32_t lastResetDay;
      
      // Helper methods
      uint32_t GetCurrentDayOfYear() const;
      bool IsNewDay() const;
    };
  }
}