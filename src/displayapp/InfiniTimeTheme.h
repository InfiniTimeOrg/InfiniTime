#pragma once

#include <lvgl/lvgl.h>
#include "displayapp/Colors.h"

namespace InfiniTimeTheme {
  namespace Colors {
    static constexpr ::Colors::Color bg = 0x5d697e;
    static constexpr ::Colors::Color bgAlt = 0x383838;
    static constexpr ::Colors::Color bgDark = 0x181818;
    static constexpr ::Colors::Color highlight = ::Colors::Green;
  };
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
