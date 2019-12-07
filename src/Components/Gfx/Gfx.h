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
        void ClearScreen();
        void DrawString(uint8_t x, uint8_t y, uint16_t color, const char* text, const FONT_INFO *p_font, bool wrap);
        void DrawChar(const FONT_INFO *font, uint8_t c, uint8_t *x, uint8_t y, uint16_t color);


      private:
        Drivers::St7789& lcd;
        const uint8_t width = 240;
        const uint8_t height = 240;
        void pixel_draw(uint8_t x, uint8_t y, uint16_t color);
    };
  }
}
