#pragma once
#include <cstdint>
#include <nrf_font.h>


namespace Pinetime {
  namespace Drivers {
    class St7789;
  }
  namespace Components {
    class Gfx {
      public:
        explicit Gfx(Drivers::St7789& lcd);
        void Init();
        void ClearScreen();
        void DrawString(uint8_t x, uint8_t y, uint16_t color, const char* text, const FONT_INFO *p_font, bool wrap);
        void DrawChar(const FONT_INFO *font, uint8_t c, uint8_t *x, uint8_t y, uint16_t color);
        void FillRectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint16_t color);

        void Sleep();
        void Wakeup();

      private:
        static constexpr uint8_t width = 240;
        static constexpr uint8_t height = 240;

        uint16_t buffer[width]; // 1 line buffer
        Drivers::St7789& lcd;

        void pixel_draw(uint8_t x, uint8_t y, uint16_t color);
        void SetBackgroundColor(uint16_t color);
    };
  }
}
