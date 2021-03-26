#pragma once
#include <cstdint>

namespace Pinetime {
  namespace Controllers {
    class Settings {
      public:
        enum class ClockType {H24, H12};

        void Init();

        void SetClockFace( uint8_t face ) { clockFace = face; };
        uint8_t GetClockFace() { return clockFace; };

        void SetAppMenu( uint8_t menu ) { appMenu = menu; };
        uint8_t GetAppMenu() { return appMenu; };

        void SetClockType( ClockType clocktype ) { clockType = clocktype; };
        ClockType GetClockType() { return clockType; };


      private:
        uint8_t clockFace = 0;
        uint8_t appMenu = 0;

        ClockType clockType = ClockType::H24;

    };
  }
}