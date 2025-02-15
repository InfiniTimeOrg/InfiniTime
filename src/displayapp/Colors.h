#pragma once

#include <lvgl/src/lv_misc/lv_color.h>

namespace Colors {

  enum Named : uint32_t {
    White = 0xFFFFFF,
    Silver = 0xC0C0C0,
    LightGray = 0xB0B0B0,
    Gray = 0x808080,
    DarkGray = 0x1B1B1B,
    Black = 0x000000,
    Red = 0xFF0000,
    Maroon = 0xB00000,
    Yellow = 0xFFFF00,
    Olive = 0xB0B000,
    Lime = 0x00FF00,
    Green = 0x00B000,
    Cyan = 0x00FFFF,
    Teal = 0x00B0B0,
    Blue = 0x0000FF,
    Navy = 0x0000B0,
    Magenta = 0xFF00FF,
    Purple = 0xB000B0,
    Orange = 0xFFA500,
    DeepOrange = 0xFF4000,
    Pink = 0xFFAEC9,
    Platinum = 0xE5E5E2,
    Gold = 0xFFD700,
    Copper = 0xB87333,
    Violet = 0x6000FF,
    Aqua = 0x00FFFF
  };

  class Color {
  public:
    constexpr Color(uint32_t color) {
      data.hexcode = color;
    }

    operator uint32_t() const {
      return data.hexcode;
    }

    constexpr Color(uint8_t r, uint8_t g, uint8_t b) {
      data.argb.red = r;
      data.argb.green = g;
      data.argb.blue = b;
    }

    operator lv_color_t() const {
      return lv_color_hex(data.hexcode);
    }

    uint8_t red() const {
      return data.argb.red;
    }

    uint8_t green() const {
      return data.argb.green;
    }

    uint8_t blue() const {
      return data.argb.blue;
    }

  private:
    union {
      uint32_t hexcode = 0;

      struct {
        uint8_t blue = 0;
        uint8_t green = 0;
        uint8_t red = 0;
        uint8_t alpha = 0;
      } argb;
    } data;
  };

  Color linear_gradient(Color startingColor, Color endingColor, uint8_t progress);
}
