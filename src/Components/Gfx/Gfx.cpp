#include <libraries/svc/nrf_svci.h>
#include "Gfx.h"
#include "../../drivers/St7789.h"
using namespace Pinetime::Components;

Gfx::Gfx(Pinetime::Drivers::St7789 &lcd) : lcd{lcd} {
  lcd.Init();
}

void Gfx::ClearScreen() {
  lcd.FillRectangle(0, 0, width, height, 0x0000);
}

void Gfx::FillRectangle(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint16_t color) {
  lcd.FillRectangle(x, y, width, height, color);
}

void Gfx::DrawString(uint8_t x, uint8_t y, uint16_t color, const char *text, const FONT_INFO *p_font, bool wrap) {
  if (y > (height - p_font->height)) {
    // Not enough space to write even single char.
    return;
  }

  uint8_t current_x = x;
  uint8_t current_y = y;

  for (size_t i = 0; text[i] != '\0'; i++) {
    if (text[i] == '\n') {
      current_x = x;
      current_y += p_font->height + p_font->height / 10;
    } else {
      DrawChar(p_font, (uint8_t) text[i], &current_x, current_y, color);
    }

    uint8_t char_idx = text[i] - p_font->startChar;
    uint16_t char_width = text[i] == ' ' ? (p_font->height / 2) : p_font->charInfo[char_idx].widthBits;

    if (current_x > (width - char_width)) {
      if (wrap) {
        current_x = x;
        current_y += p_font->height + p_font->height / 10;
      } else {
        break;
      }

      if (y > (height - p_font->height)) {
        break;
      }
    }
  }
}

void Gfx::DrawChar(const FONT_INFO *font, uint8_t c, uint8_t *x, uint8_t y, uint16_t color) {
  uint8_t char_idx = c - font->startChar;
  uint16_t bytes_in_line = CEIL_DIV(font->charInfo[char_idx].widthBits, 8);

  if (c == ' ') {
    *x += font->height / 2;
    return;
  }

  // TODO For now, LCD and SPI driver start a new transfer (cs pin + set address windows + write byte) FOR EACH PIXEL!
  // This could be improved by setting CS pin, DC pin and address window ONLY ONCE for the whole character

  lcd.BeginDrawBuffer(*x, y, bytes_in_line*8, font->height);
  uint16_t bg = 0x0000;
  for (uint16_t i = 0; i < font->height; i++) {
    for (uint16_t j = 0; j < bytes_in_line; j++) {
      for (uint8_t k = 0; k < 8; k++) {
        if ((1 << (7 - k)) &
            font->data[font->charInfo[char_idx].offset + i * bytes_in_line + j]) {
          lcd.NextDrawBuffer(reinterpret_cast<uint8_t *>(&color), 2);
        }
        else {
          lcd.NextDrawBuffer(reinterpret_cast<uint8_t *>(&bg), 2);
        }
      }
    }
  }
  lcd.EndDrawBuffer();
  *x += font->charInfo[char_idx].widthBits + font->spacePixels;
}

void Gfx::pixel_draw(uint8_t x, uint8_t y, uint16_t color) {
  lcd.DrawPixel(x, y, color);
}

void Gfx::Sleep() {
  lcd.Sleep();
}

void Gfx::Wakeup() {
  lcd.Wakeup();
}


