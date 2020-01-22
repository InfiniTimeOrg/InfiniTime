#pragma once
#include <cstddef>

namespace Pinetime {
  namespace Drivers {
    class SpiMaster;
    class St7789 {
      public:
        explicit St7789(SpiMaster& spiMaster, uint8_t pinDataCommand);
        void Init();
        void Uninit();
        void DrawPixel(uint16_t x, uint16_t y, uint32_t color);

        void BeginDrawBuffer(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
        void NextDrawBuffer(const uint8_t* data, size_t size, size_t repeat = 0);
        void EndDrawBuffer();

        void DisplayOn();
        void DisplayOff();

        void Sleep();
        void Wakeup();


      private:
        SpiMaster& spi;
        uint8_t pinDataCommand;

        void HardwareReset();
        void SoftwareReset();
        void SleepOut();
        void SleepIn();
        void ColMod();
        void MemoryDataAccessControl();
        void DisplayInversionOn();
        void NormalModeOn();
        void WriteToRam();


        void SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

        void WriteCommand(uint8_t cmd);
        void WriteSpi(const uint8_t* data, size_t size, size_t repeat = 0);

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
          MemoryDataAccessControl = 036,
          ColMod = 0x3a,
        };
        void WriteData(uint8_t data);
        void ColumnAddressSet();

        static constexpr uint16_t Width = 240;
        static constexpr uint16_t Height = 240;
        void RowAddressSet();

    };
  }
}


