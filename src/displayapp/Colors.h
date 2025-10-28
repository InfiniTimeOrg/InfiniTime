#pragma once

#include <lvgl/src/lv_misc/lv_color.h>

namespace Colors {

  class Color {
  public:
    constexpr Color(uint32_t color) : data {color} {
    }

    constexpr operator uint32_t() const {
      return data;
    }

    constexpr Color(uint8_t r, uint8_t g, uint8_t b)
      : data {(static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | static_cast<uint32_t>(b)} {
    }

    operator lv_color_t() const {
      return lv_color_hex(data);
    }

    uint8_t red() const {
      return (data & 0xFF0000) >> 16;
    }

    uint8_t green() const {
      return (data & 0x00FF00) >> 8;
    }

    uint8_t blue() const {
      return (data & 0x0000FF);
    }

  private:
    uint32_t data;
  };

  Color linear_gradient(Color startingColor, Color endingColor, uint8_t progress);

  static constexpr Color White = 0xFFFFFF;
  static constexpr Color Silver = 0xC0C0C0;
  static constexpr Color LightGray = 0xB0B0B0;
  static constexpr Color Gray = 0x808080;
  static constexpr Color DarkGray = 0x1B1B1B;
  static constexpr Color Black = 0x000000;
  static constexpr Color Red = 0xFF0000;
  static constexpr Color Maroon = 0xB00000;
  static constexpr Color Yellow = 0xFFFF00;
  static constexpr Color Olive = 0xB0B000;
  static constexpr Color Lime = 0x00FF00;
  static constexpr Color Green = 0x00B000;
  static constexpr Color Cyan = 0x00FFFF;
  static constexpr Color Teal = 0x00B0B0;
  static constexpr Color Blue = 0x0000FF;
  static constexpr Color Navy = 0x0000B0;
  static constexpr Color Magenta = 0xFF00FF;
  static constexpr Color Purple = 0xB000B0;
  static constexpr Color Orange = 0xFFA500;
  static constexpr Color DeepOrange = 0xFF4000;
  static constexpr Color Pink = 0xFFAEC9;
  static constexpr Color Platinum = 0xE5E5E2;
  static constexpr Color Gold = 0xFFD700;
  static constexpr Color Copper = 0xB87333;
  static constexpr Color Violet = 0x6000FF;
  static constexpr Color Aqua = 0x00FFF;
}
