#include <iostream>
#include <cassert>
#include <chrono>
#include <cstdint>

// Mock DateTime controller for testing
namespace Pinetime {
  namespace Controllers {
    class DateTime {
    public:
        DateTime() : currentDay(1) {}
        
        void SetDay(int day) { currentDay = day; }
        
        int DayOfYear() const { return currentDay; }
        
    private:
        int currentDay;
    };
  }
}

// Simplified SessionStatistics implementation for testing
namespace Pinetime {
  namespace Controllers {
    class SessionStatistics {
    public:
      explicit SessionStatistics(DateTime& dateTimeController)
        : dateTimeController(dateTimeController),
          dailyCompletedSessions(0),
          dailyWorkTime(std::chrono::minutes::zero()),
          lastResetDay(GetCurrentDayOfYear()) {
      }
      
      void IncrementCompletedSessions() {
        CheckAndResetIfNewDay();
        
        if (dailyCompletedSessions < UINT8_MAX) {
          dailyCompletedSessions++;
        }
      }
      
      void AddWorkTime(std::chrono::minutes duration) {
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
      
      void ResetDailyStats() {
        dailyCompletedSessions = 0;
        dailyWorkTime = std::chrono::minutes::zero();
        lastResetDay = GetCurrentDayOfYear();
      }
      
      uint8_t GetDailyCompletedSessions() const {
        return dailyCompletedSessions;
      }
      
      std::chrono::minutes GetDailyWorkTime() const {
        return dailyWorkTime;
      }
      
      void CheckAndResetIfNewDay() {
        if (IsNewDay()) {
          ResetDailyStats();
        }
      }
      
    private:
      DateTime& dateTimeController;
      uint8_t dailyCompletedSessions;
      std::chrono::minutes dailyWorkTime;
      uint32_t lastResetDay;
      
      uint32_t GetCurrentDayOfYear() const {
        return static_cast<uint32_t>(dateTimeController.DayOfYear());
      }
      
      bool IsNewDay() const {
        uint32_t currentDay = GetCurrentDayOfYear();
        
        if (currentDay < lastResetDay && (lastResetDay - currentDay) > 300) {
          return true;
        }
        
        return currentDay != lastResetDay;
      }
    };
  }
}

// Test helper functions
void assert_equal(uint8_t expected, uint8_t actual, const char* test_name) {
    if (expected != actual) {
        std::cout << "FAIL: " << test_name << " - Expected: " << (int)expected << ", Got: " << (int)actual << std::endl;
        assert(false);
    } else {
        std::cout << "PASS: " << test_name << std::endl;
    }
}

void assert_equal(std::chrono::minutes expected, std::chrono::minutes actual, const char* test_name) {
    if (expected != actual) {
        std::cout << "FAIL: " << test_name << " - Expected: " << expected.count() << " minutes, Got: " << actual.count() << " minutes" << std::endl;
        assert(false);
    } else {
        std::cout << "PASS: " << test_name << std::endl;
    }
}

// Test daily session counting logic
void test_daily_session_counting() {
    std::cout << "\n=== Testing Daily Session Counting ===" << std::endl;
    
    Pinetime::Controllers::DateTime mockDateTime;
    Pinetime::Controllers::SessionStatistics stats(mockDateTime);
    
    // Test initial state
    assert_equal(0, stats.GetDailyCompletedSessions(), "Initial session count should be 0");
    
    // Test incrementing sessions
    stats.IncrementCompletedSessions();
    assert_equal(1, stats.GetDailyCompletedSessions(), "Session count should be 1 after increment");
    
    stats.IncrementCompletedSessions();
    stats.IncrementCompletedSessions();
    assert_equal(3, stats.GetDailyCompletedSessions(), "Session count should be 3 after multiple increments");
    
    // Test overflow protection (increment to max value)
    for (int i = 3; i < 255; i++) {
        stats.IncrementCompletedSessions();
    }
    assert_equal(255, stats.GetDailyCompletedSessions(), "Session count should cap at 255");
    
    // Test that incrementing at max doesn't overflow
    stats.IncrementCompletedSessions();
    assert_equal(255, stats.GetDailyCompletedSessions(), "Session count should remain at 255 after overflow attempt");
}

// Test midnight reset behavior
void test_midnight_reset() {
    std::cout << "\n=== Testing Midnight Reset ===" << std::endl;
    
    Pinetime::Controllers::DateTime mockDateTime;
    Pinetime::Controllers::SessionStatistics stats(mockDateTime);
    
    // Set initial day and add some sessions
    mockDateTime.SetDay(1);
    stats.IncrementCompletedSessions();
    stats.IncrementCompletedSessions();
    stats.AddWorkTime(std::chrono::minutes(50));
    
    assert_equal(2, stats.GetDailyCompletedSessions(), "Should have 2 sessions on day 1");
    assert_equal(std::chrono::minutes(50), stats.GetDailyWorkTime(), "Should have 50 minutes work time on day 1");
    
    // Simulate day change
    mockDateTime.SetDay(2);
    stats.CheckAndResetIfNewDay();
    
    assert_equal(0, stats.GetDailyCompletedSessions(), "Sessions should reset to 0 on new day");
    assert_equal(std::chrono::minutes(0), stats.GetDailyWorkTime(), "Work time should reset to 0 on new day");
    
    // Test year rollover (day 365 to day 1)
    mockDateTime.SetDay(365);
    stats.IncrementCompletedSessions();
    assert_equal(1, stats.GetDailyCompletedSessions(), "Should have 1 session on day 365");
    
    mockDateTime.SetDay(1); // New year
    stats.CheckAndResetIfNewDay();
    assert_equal(0, stats.GetDailyCompletedSessions(), "Sessions should reset on year rollover");
}

// Test work time accumulation accuracy
void test_work_time_accumulation() {
    std::cout << "\n=== Testing Work Time Accumulation ===" << std::endl;
    
    Pinetime::Controllers::DateTime mockDateTime;
    Pinetime::Controllers::SessionStatistics stats(mockDateTime);
    
    // Test initial state
    assert_equal(std::chrono::minutes(0), stats.GetDailyWorkTime(), "Initial work time should be 0");
    
    // Test adding work time
    stats.AddWorkTime(std::chrono::minutes(25));
    assert_equal(std::chrono::minutes(25), stats.GetDailyWorkTime(), "Work time should be 25 minutes after adding 25");
    
    stats.AddWorkTime(std::chrono::minutes(30));
    assert_equal(std::chrono::minutes(55), stats.GetDailyWorkTime(), "Work time should be 55 minutes after adding 30 more");
    
    // Test overflow protection
    stats.AddWorkTime(std::chrono::minutes(65500)); // This will cause overflow: 55 + 65500 = 65555 > 65535
    // Should cap at UINT16_MAX minutes
    assert_equal(std::chrono::minutes(65535), stats.GetDailyWorkTime(), "Work time should cap at maximum value");
    
    // Test that adding more time at max doesn't overflow
    stats.AddWorkTime(std::chrono::minutes(100));
    assert_equal(std::chrono::minutes(65535), stats.GetDailyWorkTime(), "Work time should remain at maximum after overflow attempt");
}

// Test manual reset functionality
void test_manual_reset() {
    std::cout << "\n=== Testing Manual Reset ===" << std::endl;
    
    Pinetime::Controllers::DateTime mockDateTime;
    Pinetime::Controllers::SessionStatistics stats(mockDateTime);
    
    // Add some data
    stats.IncrementCompletedSessions();
    stats.IncrementCompletedSessions();
    stats.AddWorkTime(std::chrono::minutes(50));
    
    assert_equal(2, stats.GetDailyCompletedSessions(), "Should have 2 sessions before reset");
    assert_equal(std::chrono::minutes(50), stats.GetDailyWorkTime(), "Should have 50 minutes before reset");
    
    // Manual reset
    stats.ResetDailyStats();
    
    assert_equal(0, stats.GetDailyCompletedSessions(), "Sessions should be 0 after manual reset");
    assert_equal(std::chrono::minutes(0), stats.GetDailyWorkTime(), "Work time should be 0 after manual reset");
}

int main() {
    std::cout << "Running SessionStatistics Unit Tests" << std::endl;
    std::cout << "====================================" << std::endl;
    
    try {
        test_daily_session_counting();
        test_midnight_reset();
        test_work_time_accumulation();
        test_manual_reset();
        
        std::cout << "\n====================================" << std::endl;
        std::cout << "All tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cout << "\nTest failed with exception: " << e.what() << std::endl;
        return 1;
    }
}