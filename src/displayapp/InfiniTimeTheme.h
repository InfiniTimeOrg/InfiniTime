#pragma once

#include <lvgl/lvgl.h>

// Forward declaration
namespace Pinetime {
  namespace Controllers {
    class FS;
  }
}

namespace Colors {
  static constexpr lv_color_t deepOrange = LV_COLOR_MAKE(0xff, 0x40, 0x0);
  static constexpr lv_color_t orange = LV_COLOR_MAKE(0xff, 0xb0, 0x0);
  static constexpr lv_color_t green = LV_COLOR_MAKE(0x0, 0xb0, 0x0);
  static constexpr lv_color_t blue = LV_COLOR_MAKE(0x0, 0x50, 0xff);
  static constexpr lv_color_t lightGray = LV_COLOR_MAKE(0xb0, 0xb0, 0xb0);
  static constexpr lv_color_t gray = LV_COLOR_MAKE(0x50, 0x50, 0x50);

  // Configurable colors that can be loaded from storage
  inline lv_color_t accent_light = LV_COLOR_MAKE(0x5d, 0x69, 0x7e);
  inline lv_color_t accent = LV_COLOR_MAKE(0x38, 0x38, 0x38);
  inline lv_color_t accent_dark = LV_COLOR_MAKE(0x18, 0x18, 0x18);
  inline lv_color_t highlight = green;
  inline lv_color_t text_primary = LV_COLOR_WHITE;
  inline lv_color_t text_header = LV_COLOR_GRAY;
  inline lv_color_t page_bg = LV_COLOR_BLACK;
  inline lv_color_t icon = orange;
};

/**
 * Initialize the default theme
 * @param filesystem FS pointer to load custom colors from, can be nullptr
 * @return a pointer to reference this theme later
 */
lv_theme_t* lv_pinetime_theme_init(Pinetime::Controllers::FS* filesystem = nullptr);

/**
 * Set the filesystem reference for theme color loading
 * @param filesystem FS reference to load colors from config file
 */
void lv_pinetime_theme_set_filesystem(Pinetime::Controllers::FS* filesystem);

/**
 * Reload theme configuration from the filesystem
 * Call this after the filesystem is fully initialized if theme was initialized before
 */
void lv_pinetime_theme_reload_config();

/**
 * Check if a custom theme is currently loaded from the filesystem
 * Returns true if filesystem is set and theme.cfg file exists and was loaded
 * Returns false if using built-in colors
 */
bool lv_pinetime_theme_is_custom_loaded();
