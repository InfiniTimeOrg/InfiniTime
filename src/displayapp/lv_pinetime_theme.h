/**
 * @file lv_pinetime_theme.h
 *
 */

#ifndef LV_PINETIME_THEME_H
#define LV_PINETIME_THEME_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <lvgl/lvgl.h>

/*********************
 *      DEFINES
 *********************/
/*Colors*/
#define LV_PINETIME_GRAY        LV_COLOR_MAKE(0x8a, 0x8a, 0x8a)
#define LV_PINETIME_LIGHT_GRAY  LV_COLOR_MAKE(0xc4, 0xc4, 0xc4)
#define LV_PINETIME_BG          LV_COLOR_MAKE(0x5d, 0x69, 0x7e)
#define LV_PINETIME_BG_DISABLED LV_COLOR_MAKE(0x0b, 0x0b, 0x0b)
#define LV_PINETIME_GREEN       LV_COLOR_MAKE(0x00, 0xb0, 0x00)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

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
/**********************
 *      MACROS
 **********************/

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif
