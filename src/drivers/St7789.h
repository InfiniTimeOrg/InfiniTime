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

      void LowPowerOn();
      void LowPowerOff();
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
      void Command2Enable();
      void SleepOut();
      void EnsureSleepOutPostDelay();
      void SleepIn();
      void PixelFormat();
      void MemoryDataAccessControl();
      void DisplayInversionOn();
      void NormalModeOn();
      void WriteToRam(const uint8_t* data, size_t size);
      void IdleModeOn();
      void IdleModeOff();
      void FrameRateNormalSet();
      void IdleFrameRateOff();
      void IdleFrameRateOn();
      void DisplayOn();
      void DisplayOff();
      void PowerControl();
      void GateControl();
      void PorchSet();

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
        IdleModeOff = 0x38,
        IdleModeOn = 0x39,
        PixelFormat = 0x3a,
        FrameRateIdle = 0xb3,
        FrameRateNormal = 0xc6,
        VdvSet = 0xc4,
        Command2Enable = 0xdf,
        PowerControl1 = 0xd0,
        PowerControl2 = 0xe8,
        GateControl = 0xb7,
        Porch = 0xb2,
      };
      void WriteData(uint8_t data);
      void WriteData(const uint8_t* data, size_t size);

      static constexpr uint16_t Width = 240;
      static constexpr uint16_t Height = 320;

      uint8_t addrWindowArgs[4];
      uint8_t verticalScrollArgs[2];
    };
  }
}
