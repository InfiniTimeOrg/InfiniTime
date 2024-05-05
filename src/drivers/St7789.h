#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>

#include <FreeRTOS.h>

namespace Pinetime {
  namespace Drivers {
    class Spi;

    class St7789 {
    public:
      explicit St7789(Spi& spi, uint8_t pinDataCommand, uint8_t pinReset);
      St7789(const St7789&) = delete;
      St7789& operator=(const St7789&) = delete;
      St7789(St7789&&) = delete;
      St7789& operator=(St7789&&) = delete;

      void Init();
      void Uninit();

      void VerticalScrollStartAddress(uint16_t line);

      void DrawBuffer(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint8_t* data, size_t size);

      void Sleep();
      void Wakeup();

    private:
      Spi& spi;
      uint8_t pinDataCommand;
      uint8_t pinReset;
      uint8_t verticalScrollingStartAddress = 0;
      bool sleepIn;
      TickType_t lastSleepExit;

      void HardwareReset();
      void SoftwareReset();
      void SleepOut();
      void EnsureSleepOutPostDelay();
      void SleepIn();
      void ColMod();
      void MemoryDataAccessControl();
      void DisplayInversionOn();
      void NormalModeOn();
      void WriteToRam(const uint8_t* data, size_t size);
      void DisplayOn();
      void DisplayOff();

      void SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
      void SetVdv();
      void WriteCommand(uint8_t cmd);
      void WriteCommand(const uint8_t* data, size_t size);
      void WriteSpi(const uint8_t* data, size_t size, const std::function<void()>& preTransactionHook);

      enum class Commands : uint8_t {
        SoftwareReset = 0x01,
        SleepIn = 0x10,
        SleepOut = 0x11,
        NormalModeOn = 0x13,
        DisplayInversionOn = 0x21,
        DisplayOff = 0x28,
        DisplayOn = 0x29,
        ColumnAddressSet = 0x2a,
        RowAddressSet = 0x2b,
        WriteToRam = 0x2c,
        MemoryDataAccessControl = 0x36,
        VerticalScrollDefinition = 0x33,
        VerticalScrollStartAddress = 0x37,
        ColMod = 0x3a,
        VdvSet = 0xc4,
      };
      void WriteData(uint8_t data);
      void WriteData(const uint8_t* data, size_t size);
      void ColumnAddressSet();

      static constexpr uint16_t Width = 240;
      static constexpr uint16_t Height = 320;
      void RowAddressSet();
    };
  }
}
