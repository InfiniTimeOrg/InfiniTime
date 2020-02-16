#pragma once

#include <cstdint>
#include <chrono>
#include <Components/Gfx/Gfx.h>
#include "Screen.h"
#include <bits/unique_ptr.h>
#include "../Fonts/lcdfont14.h"
#include "../Fonts/lcdfont70.h"
#include "../../Version.h"

namespace Pinetime {
  namespace Applications {
    namespace Screens {

      template <class T>
      class DirtyValue {
        public:
          explicit DirtyValue(T v) { value = v; }
          explicit DirtyValue(T& v) { value = v; }
          bool IsUpdated() const { return isUpdated; }
          T& Get() { this->isUpdated = false; return value;}

          DirtyValue& operator=(const T& other) {
            this->value = other;
            this->isUpdated = true;
            return *this;
          }
        private:
          T value;
          bool isUpdated = true;
      };
      class Clock : public Screen{
        public:
          enum class BleConnectionStates{ NotConnected, Connected};
          Clock(Components::Gfx& gfx) : Screen(gfx), currentDateTime{{}}, version {{}} {}
          void Refresh(bool fullRefresh) override;

          void SetBatteryPercentRemaining(uint8_t percent) { batteryPercentRemaining = percent; }
          void SetBleConnectionState(BleConnectionStates state) { bleState = state; }
          void SetCurrentDateTime(const std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>& tp) { currentDateTime = tp;}

        private:
          static const char* MonthToString(Pinetime::Controllers::DateTime::Months month);
          static const char* DayOfWeekToString(Pinetime::Controllers::DateTime::Days dayOfWeek);
          static char const *DaysString[];
          static char const *MonthsString[];

          const FONT_INFO largeFont {lCD_70ptFontInfo.height, lCD_70ptFontInfo.startChar, lCD_70ptFontInfo.endChar, lCD_70ptFontInfo.spacePixels, lCD_70ptFontInfo.charInfo, lCD_70ptFontInfo.data};
          const FONT_INFO smallFont {lCD_14ptFontInfo.height, lCD_14ptFontInfo.startChar, lCD_14ptFontInfo.endChar, lCD_14ptFontInfo.spacePixels, lCD_14ptFontInfo.charInfo, lCD_14ptFontInfo.data};

          char currentChar[4];
          uint16_t currentYear = 1970;
          Pinetime::Controllers::DateTime::Months currentMonth = Pinetime::Controllers::DateTime::Months::Unknown;
          Pinetime::Controllers::DateTime::Days currentDayOfWeek = Pinetime::Controllers::DateTime::Days::Unknown;
          uint8_t currentDay = 0;

          DirtyValue<uint8_t> batteryPercentRemaining  {0};
          DirtyValue<BleConnectionStates> bleState {BleConnectionStates::NotConnected};
          DirtyValue<std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds>> currentDateTime;
          DirtyValue<Pinetime::Version> version;
      };
    }
  }
}
