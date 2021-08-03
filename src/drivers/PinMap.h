#pragma once

namespace Pinetime {
  namespace PinMap {
    #define WATCH_P8
    #ifdef WATCH_P8
      static constexpr uint8_t Charging = 19;
      static constexpr uint8_t Cst816sReset = 13;
      static constexpr uint8_t Button = 17;
    #else
      static constexpr uint8_t Charging = 12;
      static constexpr uint8_t Cst816sReset = 10;
      static constexpr uint8_t Button = 13;      
    #endif

       static constexpr uint8_t Cst816sIrq = 28;
       static constexpr uint8_t PowerPresent = 19;

       static constexpr uint8_t Motor = 16;

       static constexpr uint8_t LcdBacklight1 = 14;
       static constexpr uint8_t LcdBacklight2 = 22;
       static constexpr uint8_t LcdBacklight3 = 23;

       static constexpr uint8_t SpiSck = 2;
       static constexpr uint8_t SpiMosi = 3;
       static constexpr uint8_t SpiMiso = 4;

       static constexpr uint8_t SpiFlashCsn = 5;
       static constexpr uint8_t SpiLcdCsn = 25;
       static constexpr uint8_t LcdDataCommand = 18;

       static constexpr uint8_t TwiScl = 7;
       static constexpr uint8_t TwiSda = 6;

  }
}


#ifdef WATCH_P8

// BatteryController.h
#define PINMAP_CHARGING_PIN 19

// Cst816s.h
#define PINMAP_CST816S_RESET_PIN 13

// SystemTask.h
#define PINMAP_BUTTON_PIN 17

#else

// BatteryController.h
#define PINMAP_CHARGING_PIN 12

// Cst816s.h
#define PINMAP_CST816S_RESET_PIN 10

// SystemTask.h
#define PINMAP_BUTTON_PIN 13

#endif