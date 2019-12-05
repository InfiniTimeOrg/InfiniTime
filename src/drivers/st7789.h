#pragma once

namespace Pinetime {
  namespace Drivers {
    class st7789 {
      public:
        ret_code_t Init();
        void Uninit();
        void DrawPixel(uint16_t x, uint16_t y, uint32_t color);
        void DrawRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color);
        void FillRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);

      private:
        ret_code_t InitHw() const;
        void InitCommands();

        void SoftwareReset();
        void SleepOut();
        void ColMod();
        void MemoryDataAccessControl();
        void DisplayInversionOn();
        void NormalModeOn();
        void WriteToRam();
        void DisplayOn();
        void DisplayOff();

        void SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

        void WriteCommand(uint8_t cmd);
        void WriteSpi(const uint8_t* data, size_t size);

        enum class Commands : uint8_t {
          SoftwareReset = 0x01,
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


