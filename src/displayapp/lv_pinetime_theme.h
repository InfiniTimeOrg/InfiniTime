#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <lvgl/lvgl.h>

#define IT_COLOR_BG      LV_COLOR_MAKE(0x5d, 0x69, 0x7e)
#define IT_COLOR_BG_DARK LV_COLOR_MAKE(0x18, 0x18, 0x18)
#define IT_COLOR_SEL     LV_COLOR_MAKE(0x0, 0xb0, 0x0)

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
lv_theme_t* lv_pinetime_theme_init(lv_color_t color_primary,
                                   lv_color_t color_secondary,
                                   uint32_t flags,
                                   const lv_font_t* font_small,
                                   const lv_font_t* font_normal,
                                   const lv_font_t* font_subtitle,
                                   const lv_font_t* font_title);
#ifdef __cplusplus
} /* extern "C" */
#endif
