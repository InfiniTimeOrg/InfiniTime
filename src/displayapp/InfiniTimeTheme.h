#pragma once

#include <lvgl/lvgl.h>

namespace Colors {
  static constexpr lv_color_t deepOrange = LV_COLOR_MAKE(0xd6, 0x5d, 0x0e);
  static constexpr lv_color_t orange = LV_COLOR_MAKE(0xfe, 0x80, 0x19);
  static constexpr lv_color_t green = LV_COLOR_MAKE(0x98, 0x97, 0x1a);
  static constexpr lv_color_t blue = LV_COLOR_MAKE(0x45, 0x85, 0x88);
  static constexpr lv_color_t lightGray = LV_COLOR_MAKE(0xa8, 0x99, 0x84);

  static constexpr lv_color_t bg = LV_COLOR_MAKE(0x50, 0x49, 0x45);
  static constexpr lv_color_t bgAlt = LV_COLOR_MAKE(0x3c, 0x38, 0x36);
  static constexpr lv_color_t bgDark = LV_COLOR_MAKE(0x28, 0x28, 0x28);
  static constexpr lv_color_t highlight = blue;
};

/**
 * Initialize the default
 * @param color_primary the primary color of the theme
 * @param color_secondary the secondary color for the theme
 * @param flags ORed flags starting with `LV_THEME_DEF_FLAG_...`
 * @param font_small pointer to a small font
 * @param font_normal pointer to a normal font
 * @param font_subtitle pointer to a large font
 * @param font_title pointer to a extra large font
 * @return a pointer to reference this theme later
 */
lv_theme_t* lv_pinetime_theme_init();
