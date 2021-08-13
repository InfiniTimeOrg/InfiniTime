/*******************************************************************************
 * Size: 40 px
 * Bpp: 1
 * Opts: 
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef JETBRAINS_MONO_40
#define JETBRAINS_MONO_40 1
#endif

#if JETBRAINS_MONO_40

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+002D "-" */
    0xff, 0xff, 0xff, 0xff, 0xfe,

    /* U+0030 "0" */
    0x3, 0xf0, 0x7, 0xff, 0x83, 0xff, 0xf1, 0xf8,
    0x7e, 0x78, 0x7, 0xbe, 0x1, 0xff, 0x0, 0x3f,
    0xc0, 0xf, 0xf0, 0x3, 0xfc, 0x0, 0xff, 0x0,
    0x3f, 0xc0, 0xf, 0xf1, 0xc3, 0xfc, 0xf8, 0xff,
    0x3e, 0x3f, 0xcf, 0x8f, 0xf1, 0xc3, 0xfc, 0x0,
    0xff, 0x0, 0x3f, 0xc0, 0xf, 0xf0, 0x3, 0xfc,
    0x0, 0xff, 0x0, 0x3f, 0xc0, 0xf, 0xf8, 0x7,
    0xde, 0x1, 0xe7, 0xe1, 0xf8, 0xff, 0xfc, 0x1f,
    0xfe, 0x1, 0xfe, 0x0,

    /* U+0031 "1" */
    0x3, 0xf0, 0x1, 0xfc, 0x1, 0xff, 0x0, 0xff,
    0xc0, 0x7c, 0xf0, 0x3e, 0x3c, 0xf, 0xf, 0x3,
    0x3, 0xc0, 0x80, 0xf0, 0x0, 0x3c, 0x0, 0xf,
    0x0, 0x3, 0xc0, 0x0, 0xf0, 0x0, 0x3c, 0x0,
    0xf, 0x0, 0x3, 0xc0, 0x0, 0xf0, 0x0, 0x3c,
    0x0, 0xf, 0x0, 0x3, 0xc0, 0x0, 0xf0, 0x0,
    0x3c, 0x0, 0xf, 0x0, 0x3, 0xc0, 0x0, 0xf0,
    0x0, 0x3c, 0x0, 0xf, 0x3, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xf0,

    /* U+0032 "2" */
    0x3, 0xf0, 0x3, 0xff, 0x3, 0xff, 0xf1, 0xf8,
    0x7e, 0x78, 0x7, 0xbe, 0x1, 0xff, 0x0, 0x3f,
    0xc0, 0xf, 0xf0, 0x3, 0xc0, 0x0, 0xf0, 0x0,
    0x3c, 0x0, 0x1f, 0x0, 0x7, 0x80, 0x3, 0xe0,
    0x1, 0xf0, 0x0, 0x78, 0x0, 0x3e, 0x0, 0x1f,
    0x0, 0xf, 0x80, 0x7, 0xc0, 0x3, 0xe0, 0x1,
    0xf0, 0x0, 0x78, 0x0, 0x3c, 0x0, 0x1f, 0x0,
    0xf, 0x80, 0x7, 0xc0, 0x1, 0xff, 0xff, 0x7f,
    0xff, 0xdf, 0xff, 0xf0,

    /* U+0033 "3" */
    0x7f, 0xff, 0x1f, 0xff, 0xc7, 0xff, 0xf0, 0x0,
    0x3c, 0x0, 0x1e, 0x0, 0x7, 0x0, 0x3, 0x80,
    0x1, 0xe0, 0x0, 0xf0, 0x0, 0x78, 0x0, 0x3c,
    0x0, 0xf, 0xe0, 0x3, 0xfe, 0x0, 0xff, 0xc0,
    0x1, 0xf8, 0x0, 0x1e, 0x0, 0x7, 0xc0, 0x0,
    0xf0, 0x0, 0x3c, 0x0, 0xf, 0x0, 0x3, 0xc0,
    0x0, 0xff, 0x0, 0x3f, 0xc0, 0xf, 0xf8, 0x7,
    0xde, 0x1, 0xe7, 0xe1, 0xf0, 0xff, 0xfc, 0x1f,
    0xfc, 0x0, 0xfc, 0x0,

    /* U+0034 "4" */
    0x0, 0x3c, 0x0, 0x1e, 0x0, 0x1e, 0x0, 0x1f,
    0x0, 0xf, 0x0, 0xf, 0x0, 0x7, 0x80, 0x7,
    0x80, 0x7, 0xc0, 0x3, 0xc0, 0x3, 0xe0, 0x1,
    0xe0, 0x1, 0xe0, 0x1, 0xf0, 0x3c, 0xf0, 0x1e,
    0xf8, 0xf, 0x78, 0x7, 0xfc, 0x3, 0xfc, 0x1,
    0xfe, 0x0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xfe, 0x0, 0xf, 0x0, 0x7, 0x80, 0x3, 0xc0,
    0x1, 0xe0, 0x0, 0xf0, 0x0, 0x78, 0x0, 0x3c,

    /* U+0035 "5" */
    0x7f, 0xff, 0x9f, 0xff, 0xe7, 0xff, 0xf9, 0xe0,
    0x0, 0x78, 0x0, 0x1e, 0x0, 0x7, 0x80, 0x1,
    0xe0, 0x0, 0x78, 0x0, 0x1e, 0x0, 0x7, 0x8f,
    0x81, 0xef, 0xf8, 0x7f, 0xff, 0x1f, 0x83, 0xe7,
    0xc0, 0x79, 0xe0, 0x1f, 0x0, 0x3, 0xc0, 0x0,
    0xf0, 0x0, 0x3c, 0x0, 0xf, 0x0, 0x3, 0xc0,
    0x0, 0xf0, 0x0, 0x3f, 0xc0, 0xf, 0x78, 0x7,
    0xde, 0x1, 0xe7, 0xe0, 0xf8, 0xff, 0xfc, 0xf,
    0xfe, 0x0, 0xfe, 0x0,

    /* U+0036 "6" */
    0x0, 0x78, 0x0, 0xe, 0x0, 0x3, 0xc0, 0x0,
    0x70, 0x0, 0x1e, 0x0, 0x7, 0x80, 0x0, 0xf0,
    0x0, 0x3c, 0x0, 0x7, 0x80, 0x1, 0xe0, 0x0,
    0x38, 0x0, 0xf, 0x7e, 0x1, 0xdf, 0xf0, 0x7f,
    0xff, 0xf, 0x83, 0xf3, 0xe0, 0x3e, 0x78, 0x3,
    0xdf, 0x0, 0x7f, 0xc0, 0x7, 0xf8, 0x0, 0xff,
    0x0, 0x1f, 0xe0, 0x3, 0xfc, 0x0, 0x7f, 0x80,
    0x1f, 0x78, 0x3, 0xcf, 0x80, 0xf8, 0xf8, 0x3e,
    0xf, 0xff, 0x80, 0xff, 0xe0, 0x7, 0xf0, 0x0,

    /* U+0037 "7" */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8,
    0x0, 0xef, 0x0, 0x3d, 0xe0, 0x7, 0xbc, 0x0,
    0xe7, 0x80, 0x3c, 0x0, 0x7, 0x80, 0x1, 0xe0,
    0x0, 0x3c, 0x0, 0x7, 0x0, 0x1, 0xe0, 0x0,
    0x3c, 0x0, 0x7, 0x0, 0x1, 0xe0, 0x0, 0x3c,
    0x0, 0xf, 0x0, 0x1, 0xe0, 0x0, 0x38, 0x0,
    0xf, 0x0, 0x1, 0xe0, 0x0, 0x38, 0x0, 0xf,
    0x0, 0x1, 0xe0, 0x0, 0x78, 0x0, 0xf, 0x0,
    0x1, 0xe0, 0x0, 0x78, 0x0, 0xf, 0x0, 0x0,

    /* U+0038 "8" */
    0x3, 0xf0, 0x3, 0xff, 0xc0, 0xff, 0xfc, 0x3f,
    0xf, 0xc7, 0x80, 0x79, 0xe0, 0x7, 0xbc, 0x0,
    0xf7, 0x80, 0x1e, 0xf0, 0x3, 0xde, 0x0, 0x79,
    0xe0, 0x1e, 0x1e, 0x7, 0x81, 0xf3, 0xe0, 0x1f,
    0xf8, 0x1, 0xfe, 0x0, 0x7f, 0xf0, 0x3e, 0x1f,
    0x7, 0x80, 0xf1, 0xe0, 0xf, 0x38, 0x1, 0xef,
    0x0, 0x1f, 0xe0, 0x3, 0xfc, 0x0, 0x7f, 0x80,
    0xf, 0xf8, 0x3, 0xef, 0x0, 0x79, 0xf8, 0x3f,
    0x1f, 0xff, 0xc1, 0xff, 0xf0, 0x7, 0xf0, 0x0,

    /* U+0039 "9" */
    0x3, 0xf8, 0x1, 0xff, 0xc0, 0x7f, 0xfc, 0x1f,
    0x7, 0xc7, 0xc0, 0x7c, 0xf0, 0x7, 0xbe, 0x0,
    0xff, 0x80, 0xf, 0xf0, 0x1, 0xfe, 0x0, 0x3f,
    0xc0, 0x7, 0xf8, 0x0, 0xff, 0x0, 0x3e, 0xf0,
    0x7, 0x9f, 0x1, 0xf3, 0xf0, 0x7c, 0x3f, 0xff,
    0x83, 0xfe, 0xe0, 0x1f, 0xbc, 0x0, 0xf, 0x0,
    0x1, 0xe0, 0x0, 0x78, 0x0, 0xf, 0x0, 0x3,
    0xc0, 0x0, 0x78, 0x0, 0x1e, 0x0, 0x3, 0x80,
    0x0, 0xf0, 0x0, 0x1c, 0x0, 0x7, 0x80, 0x0,

    /* U+0041 "A" */
    0x1, 0xf8, 0x0, 0x1f, 0x80, 0x1, 0xf8, 0x0,
    0x1f, 0x80, 0x3, 0xfc, 0x0, 0x3f, 0xc0, 0x3,
    0x9c, 0x0, 0x39, 0xc0, 0x7, 0x9e, 0x0, 0x79,
    0xe0, 0x7, 0xe, 0x0, 0x70, 0xe0, 0xf, 0xf,
    0x0, 0xf0, 0xf0, 0xe, 0x7, 0x0, 0xe0, 0x70,
    0x1e, 0x7, 0x81, 0xe0, 0x78, 0x1c, 0x3, 0x81,
    0xff, 0xf8, 0x3f, 0xff, 0xc3, 0xff, 0xfc, 0x38,
    0x1, 0xc3, 0x80, 0x1c, 0x78, 0x1, 0xe7, 0x80,
    0x1e, 0x78, 0x0, 0xe7, 0x0, 0xe, 0xf0, 0x0,
    0xff, 0x0, 0xf,

    /* U+0042 "B" */
    0xff, 0xf0, 0x3f, 0xff, 0xf, 0xff, 0xe3, 0xc0,
    0x7c, 0xf0, 0xf, 0x3c, 0x3, 0xef, 0x0, 0x7b,
    0xc0, 0x1e, 0xf0, 0x7, 0xbc, 0x1, 0xef, 0x0,
    0xfb, 0xc0, 0x3c, 0xf0, 0x1e, 0x3f, 0xff, 0xf,
    0xff, 0x83, 0xff, 0xf8, 0xf0, 0x1f, 0x3c, 0x3,
    0xef, 0x0, 0x7b, 0xc0, 0xf, 0xf0, 0x3, 0xfc,
    0x0, 0xff, 0x0, 0x3f, 0xc0, 0xf, 0xf0, 0x7,
    0xfc, 0x1, 0xef, 0x1, 0xfb, 0xff, 0xfc, 0xff,
    0xfe, 0x3f, 0xfe, 0x0,

    /* U+0043 "C" */
    0x7, 0xf0, 0xf, 0xfe, 0xf, 0xff, 0x8f, 0x83,
    0xe7, 0x80, 0xf7, 0x80, 0x7f, 0xc0, 0x1f, 0xe0,
    0xf, 0xf0, 0x0, 0x78, 0x0, 0x3c, 0x0, 0x1e,
    0x0, 0xf, 0x0, 0x7, 0x80, 0x3, 0xc0, 0x1,
    0xe0, 0x0, 0xf0, 0x0, 0x78, 0x0, 0x3c, 0x0,
    0x1e, 0x0, 0xf, 0x0, 0x7, 0x80, 0x3, 0xc0,
    0x1f, 0xe0, 0xf, 0xf0, 0xf, 0xbc, 0x7, 0x9f,
    0x7, 0xc7, 0xff, 0xc1, 0xff, 0xc0, 0x3f, 0x80,

    /* U+0044 "D" */
    0xff, 0xe0, 0x7f, 0xfc, 0x3f, 0xff, 0x9e, 0x7,
    0xcf, 0x0, 0xf7, 0x80, 0x7b, 0xc0, 0x1f, 0xe0,
    0xf, 0xf0, 0x7, 0xf8, 0x3, 0xfc, 0x1, 0xfe,
    0x0, 0xff, 0x0, 0x7f, 0x80, 0x3f, 0xc0, 0x1f,
    0xe0, 0xf, 0xf0, 0x7, 0xf8, 0x3, 0xfc, 0x1,
    0xfe, 0x0, 0xff, 0x0, 0x7f, 0x80, 0x3f, 0xc0,
    0x1f, 0xe0, 0xf, 0xf0, 0xf, 0x78, 0x7, 0xbc,
    0xf, 0x9f, 0xff, 0xcf, 0xff, 0x87, 0xff, 0x0,

    /* U+0045 "E" */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x0,
    0xf, 0x0, 0x7, 0x80, 0x3, 0xc0, 0x1, 0xe0,
    0x0, 0xf0, 0x0, 0x78, 0x0, 0x3c, 0x0, 0x1e,
    0x0, 0xf, 0x0, 0x7, 0xff, 0xfb, 0xff, 0xfd,
    0xff, 0xfe, 0xf0, 0x0, 0x78, 0x0, 0x3c, 0x0,
    0x1e, 0x0, 0xf, 0x0, 0x7, 0x80, 0x3, 0xc0,
    0x1, 0xe0, 0x0, 0xf0, 0x0, 0x78, 0x0, 0x3c,
    0x0, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc,

    /* U+0046 "F" */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x0,
    0xf, 0x0, 0x7, 0x80, 0x3, 0xc0, 0x1, 0xe0,
    0x0, 0xf0, 0x0, 0x78, 0x0, 0x3c, 0x0, 0x1e,
    0x0, 0xf, 0x0, 0x7, 0x80, 0x3, 0xff, 0xfd,
    0xff, 0xfe, 0xff, 0xff, 0x78, 0x0, 0x3c, 0x0,
    0x1e, 0x0, 0xf, 0x0, 0x7, 0x80, 0x3, 0xc0,
    0x1, 0xe0, 0x0, 0xf0, 0x0, 0x78, 0x0, 0x3c,
    0x0, 0x1e, 0x0, 0xf, 0x0, 0x7, 0x80, 0x0,

    /* U+0047 "G" */
    0x7, 0xf0, 0xf, 0xfe, 0xf, 0xff, 0x8f, 0x83,
    0xe7, 0x80, 0xf7, 0x80, 0x3f, 0xc0, 0x1f, 0xe0,
    0xf, 0xf0, 0x0, 0x78, 0x0, 0x3c, 0x0, 0x1e,
    0x0, 0xf, 0x0, 0x7, 0x80, 0x3, 0xc3, 0xff,
    0xe1, 0xff, 0xf0, 0xff, 0xf8, 0x3, 0xfc, 0x1,
    0xfe, 0x0, 0xff, 0x0, 0x7f, 0x80, 0x3f, 0xc0,
    0x1f, 0xe0, 0xf, 0xf8, 0xf, 0xbc, 0x7, 0x9f,
    0x7, 0xc7, 0xff, 0xc1, 0xff, 0xc0, 0x3f, 0x80,

    /* U+0048 "H" */
    0xf0, 0x7, 0xf8, 0x3, 0xfc, 0x1, 0xfe, 0x0,
    0xff, 0x0, 0x7f, 0x80, 0x3f, 0xc0, 0x1f, 0xe0,
    0xf, 0xf0, 0x7, 0xf8, 0x3, 0xfc, 0x1, 0xfe,
    0x0, 0xff, 0x0, 0x7f, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xf0, 0x7, 0xf8, 0x3, 0xfc, 0x1,
    0xfe, 0x0, 0xff, 0x0, 0x7f, 0x80, 0x3f, 0xc0,
    0x1f, 0xe0, 0xf, 0xf0, 0x7, 0xf8, 0x3, 0xfc,
    0x1, 0xfe, 0x0, 0xff, 0x0, 0x7f, 0x80, 0x3c,

    /* U+0049 "I" */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3, 0xc0,
    0x3, 0xc0, 0x3, 0xc0, 0x3, 0xc0, 0x3, 0xc0,
    0x3, 0xc0, 0x3, 0xc0, 0x3, 0xc0, 0x3, 0xc0,
    0x3, 0xc0, 0x3, 0xc0, 0x3, 0xc0, 0x3, 0xc0,
    0x3, 0xc0, 0x3, 0xc0, 0x3, 0xc0, 0x3, 0xc0,
    0x3, 0xc0, 0x3, 0xc0, 0x3, 0xc0, 0x3, 0xc0,
    0x3, 0xc0, 0x3, 0xc0, 0x3, 0xc0, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,

    /* U+004A "J" */
    0x3, 0xff, 0xc0, 0xff, 0xf0, 0x3f, 0xfc, 0x0,
    0xf, 0x0, 0x3, 0xc0, 0x0, 0xf0, 0x0, 0x3c,
    0x0, 0xf, 0x0, 0x3, 0xc0, 0x0, 0xf0, 0x0,
    0x3c, 0x0, 0xf, 0x0, 0x3, 0xc0, 0x0, 0xf0,
    0x0, 0x3c, 0x0, 0xf, 0x0, 0x3, 0xc0, 0x0,
    0xf0, 0x0, 0x3c, 0x0, 0xf, 0x0, 0x3, 0xfc,
    0x0, 0xff, 0x0, 0x3f, 0xc0, 0xf, 0xf8, 0x7,
    0x9e, 0x1, 0xe7, 0xe1, 0xf0, 0xff, 0xfc, 0x1f,
    0xfc, 0x0, 0xfc, 0x0,

    /* U+004B "K" */
    0xf0, 0x1, 0xde, 0x0, 0x7b, 0xc0, 0xe, 0x78,
    0x3, 0xcf, 0x0, 0x71, 0xe0, 0x1e, 0x3c, 0x3,
    0x87, 0x80, 0xf0, 0xf0, 0x1c, 0x1e, 0x7, 0x83,
    0xc0, 0xe0, 0x78, 0x3c, 0xf, 0x7, 0x1, 0xff,
    0xe0, 0x3f, 0xfc, 0x7, 0xff, 0x80, 0xf0, 0xf8,
    0x1e, 0xf, 0x3, 0xc1, 0xe0, 0x78, 0x1e, 0xf,
    0x3, 0xc1, 0xe0, 0x3c, 0x3c, 0x7, 0x87, 0x80,
    0x78, 0xf0, 0xf, 0x1e, 0x0, 0xf3, 0xc0, 0x1e,
    0x78, 0x1, 0xef, 0x0, 0x3d, 0xe0, 0x3, 0xc0,

    /* U+004C "L" */
    0xf0, 0x0, 0x78, 0x0, 0x3c, 0x0, 0x1e, 0x0,
    0xf, 0x0, 0x7, 0x80, 0x3, 0xc0, 0x1, 0xe0,
    0x0, 0xf0, 0x0, 0x78, 0x0, 0x3c, 0x0, 0x1e,
    0x0, 0xf, 0x0, 0x7, 0x80, 0x3, 0xc0, 0x1,
    0xe0, 0x0, 0xf0, 0x0, 0x78, 0x0, 0x3c, 0x0,
    0x1e, 0x0, 0xf, 0x0, 0x7, 0x80, 0x3, 0xc0,
    0x1, 0xe0, 0x0, 0xf0, 0x0, 0x78, 0x0, 0x3c,
    0x0, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc,

    /* U+004D "M" */
    0xf8, 0x7, 0xff, 0x3, 0xff, 0xc0, 0xff, 0xf0,
    0x3f, 0xec, 0xd, 0xfb, 0x87, 0x7e, 0xe1, 0xdf,
    0xd8, 0x6f, 0xf7, 0x3b, 0xfd, 0xce, 0xff, 0x73,
    0xbf, 0xcc, 0xcf, 0xf3, 0xf3, 0xfc, 0xfc, 0xff,
    0x1e, 0x3f, 0xc7, 0x8f, 0xf1, 0xe3, 0xfc, 0x0,
    0xff, 0x0, 0x3f, 0xc0, 0xf, 0xf0, 0x3, 0xfc,
    0x0, 0xff, 0x0, 0x3f, 0xc0, 0xf, 0xf0, 0x3,
    0xfc, 0x0, 0xff, 0x0, 0x3f, 0xc0, 0xf, 0xf0,
    0x3, 0xfc, 0x0, 0xf0,

    /* U+004E "N" */
    0xf8, 0x7, 0xfe, 0x3, 0xff, 0x1, 0xff, 0x80,
    0xff, 0xe0, 0x7f, 0xf0, 0x3f, 0xf8, 0x1f, 0xee,
    0xf, 0xf7, 0x7, 0xfb, 0x83, 0xfc, 0xe1, 0xfe,
    0x70, 0xff, 0x38, 0x7f, 0x8e, 0x3f, 0xc7, 0x1f,
    0xe3, 0x8f, 0xf0, 0xc7, 0xf8, 0x73, 0xfc, 0x39,
    0xfe, 0x1c, 0xff, 0x7, 0x7f, 0x83, 0xbf, 0xc1,
    0xdf, 0xe0, 0x7f, 0xf0, 0x3f, 0xf8, 0x1f, 0xfc,
    0x7, 0xfe, 0x3, 0xff, 0x1, 0xff, 0x80, 0x7c,

    /* U+004F "O" */
    0x7, 0xf0, 0xf, 0xfe, 0xf, 0xff, 0x8f, 0x83,
    0xe7, 0x80, 0xf7, 0x80, 0x7f, 0xc0, 0x1f, 0xe0,
    0xf, 0xf0, 0x7, 0xf8, 0x3, 0xfc, 0x1, 0xfe,
    0x0, 0xff, 0x0, 0x7f, 0x80, 0x3f, 0xc0, 0x1f,
    0xe0, 0xf, 0xf0, 0x7, 0xf8, 0x3, 0xfc, 0x1,
    0xfe, 0x0, 0xff, 0x0, 0x7f, 0x80, 0x3f, 0xc0,
    0x1f, 0xe0, 0xf, 0xf0, 0xf, 0xbc, 0x7, 0x9f,
    0x7, 0xc7, 0xff, 0xc1, 0xff, 0xc0, 0x3f, 0x80,

    /* U+0050 "P" */
    0xff, 0xf0, 0x3f, 0xff, 0xf, 0xff, 0xf3, 0xc0,
    0x7c, 0xf0, 0x7, 0xbc, 0x1, 0xef, 0x0, 0x3f,
    0xc0, 0xf, 0xf0, 0x3, 0xfc, 0x0, 0xff, 0x0,
    0x3f, 0xc0, 0xf, 0xf0, 0x7, 0xbc, 0x1, 0xef,
    0x1, 0xfb, 0xff, 0xfc, 0xff, 0xfe, 0x3f, 0xfe,
    0xf, 0x0, 0x3, 0xc0, 0x0, 0xf0, 0x0, 0x3c,
    0x0, 0xf, 0x0, 0x3, 0xc0, 0x0, 0xf0, 0x0,
    0x3c, 0x0, 0xf, 0x0, 0x3, 0xc0, 0x0, 0xf0,
    0x0, 0x3c, 0x0, 0x0,

    /* U+0051 "Q" */
    0x3, 0xf0, 0x7, 0xff, 0x83, 0xff, 0xf1, 0xf8,
    0x7e, 0x78, 0x7, 0xbe, 0x1, 0xff, 0x0, 0x3f,
    0xc0, 0xf, 0xf0, 0x3, 0xfc, 0x0, 0xff, 0x0,
    0x3f, 0xc0, 0xf, 0xf0, 0x3, 0xfc, 0x0, 0xff,
    0x0, 0x3f, 0xc0, 0xf, 0xf0, 0x3, 0xfc, 0x0,
    0xff, 0x0, 0x3f, 0xc0, 0xf, 0xf0, 0x3, 0xfc,
    0x0, 0xff, 0x0, 0x3f, 0xc0, 0xf, 0xf8, 0x7,
    0xde, 0x1, 0xe7, 0xe1, 0xf0, 0xff, 0xfc, 0x1f,
    0xfc, 0x1, 0xfe, 0x0, 0x3, 0xc0, 0x0, 0xf8,
    0x0, 0x1e, 0x0, 0x3, 0xc0, 0x0, 0xf8, 0x0,
    0x1e, 0x0, 0x7, 0xc0,

    /* U+0052 "R" */
    0xff, 0xf0, 0x3f, 0xff, 0x8f, 0xff, 0xf3, 0xc0,
    0x7e, 0xf0, 0x7, 0xbc, 0x1, 0xff, 0x0, 0x3f,
    0xc0, 0xf, 0xf0, 0x3, 0xfc, 0x0, 0xff, 0x0,
    0x3f, 0xc0, 0x1f, 0xf0, 0x7, 0xbc, 0x7, 0xef,
    0xff, 0xf3, 0xff, 0xf8, 0xff, 0xf8, 0x3c, 0x1c,
    0xf, 0x7, 0x83, 0xc0, 0xe0, 0xf0, 0x3c, 0x3c,
    0xf, 0xf, 0x1, 0xe3, 0xc0, 0x78, 0xf0, 0xf,
    0x3c, 0x3, 0xcf, 0x0, 0xfb, 0xc0, 0x1e, 0xf0,
    0x7, 0xfc, 0x0, 0xf0,

    /* U+0053 "S" */
    0x3, 0xf8, 0x3, 0xff, 0x81, 0xff, 0xf0, 0xf8,
    0x3e, 0x3c, 0x7, 0x9e, 0x0, 0xf7, 0x80, 0x3d,
    0xe0, 0xf, 0x78, 0x0, 0x1e, 0x0, 0x7, 0xc0,
    0x0, 0xf8, 0x0, 0x3f, 0xc0, 0x7, 0xfc, 0x0,
    0xff, 0xc0, 0xf, 0xfc, 0x0, 0x7f, 0x0, 0x7,
    0xe0, 0x0, 0x78, 0x0, 0x1f, 0x0, 0x3, 0xc0,
    0x0, 0xff, 0x0, 0x3f, 0xc0, 0xf, 0xf8, 0x7,
    0xde, 0x1, 0xe7, 0xe1, 0xf8, 0xff, 0xfc, 0x1f,
    0xfe, 0x1, 0xfe, 0x0,

    /* U+0054 "T" */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0,
    0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0, 0x0,
    0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf,
    0x0, 0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0,
    0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0,
    0xf, 0x0, 0x0, 0xf0, 0x0, 0xf, 0x0, 0x0,
    0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0, 0x0, 0xf,
    0x0, 0x0, 0xf0, 0x0, 0xf, 0x0, 0x0, 0xf0,
    0x0, 0xf, 0x0,

    /* U+0055 "U" */
    0xf0, 0x7, 0xf8, 0x3, 0xfc, 0x1, 0xfe, 0x0,
    0xff, 0x0, 0x7f, 0x80, 0x3f, 0xc0, 0x1f, 0xe0,
    0xf, 0xf0, 0x7, 0xf8, 0x3, 0xfc, 0x1, 0xfe,
    0x0, 0xff, 0x0, 0x7f, 0x80, 0x3f, 0xc0, 0x1f,
    0xe0, 0xf, 0xf0, 0x7, 0xf8, 0x3, 0xfc, 0x1,
    0xfe, 0x0, 0xff, 0x0, 0x7f, 0x80, 0x3f, 0xc0,
    0x1f, 0xe0, 0xf, 0xf8, 0xf, 0xbc, 0x7, 0x9f,
    0x7, 0xc7, 0xff, 0xc1, 0xff, 0xc0, 0x3f, 0x80,

    /* U+0056 "V" */
    0xf0, 0x0, 0xff, 0x0, 0xf, 0x70, 0x0, 0xe7,
    0x80, 0x1e, 0x78, 0x1, 0xe7, 0x80, 0x1e, 0x38,
    0x1, 0xc3, 0xc0, 0x3c, 0x3c, 0x3, 0xc3, 0xc0,
    0x3c, 0x1c, 0x3, 0x81, 0xe0, 0x38, 0x1e, 0x7,
    0x81, 0xe0, 0x78, 0xe, 0x7, 0x0, 0xe0, 0x70,
    0xf, 0xf, 0x0, 0xf0, 0xf0, 0x7, 0xe, 0x0,
    0x70, 0xe0, 0x7, 0x9e, 0x0, 0x79, 0xe0, 0x3,
    0x9c, 0x0, 0x39, 0xc0, 0x3, 0xdc, 0x0, 0x3f,
    0xc0, 0x1, 0xf8, 0x0, 0x1f, 0x80, 0x1, 0xf8,
    0x0, 0x1f, 0x80,

    /* U+0057 "W" */
    0xf0, 0x78, 0x1f, 0xc1, 0xe0, 0x7f, 0x7, 0xc1,
    0xdc, 0x1f, 0x7, 0x70, 0x7c, 0x1d, 0xc1, 0xb0,
    0xf7, 0x6, 0xc3, 0xdc, 0x3b, 0xe, 0x70, 0xec,
    0x39, 0xe3, 0xb0, 0xe7, 0x8c, 0xc3, 0x9e, 0x33,
    0x8e, 0x38, 0xce, 0x38, 0xe3, 0x38, 0xe3, 0x8c,
    0x67, 0x8e, 0x71, 0x9e, 0x39, 0xc6, 0x70, 0xe7,
    0x19, 0xc3, 0x98, 0x67, 0xf, 0x61, 0xdc, 0x3d,
    0x87, 0x70, 0x76, 0x1d, 0xc1, 0xf8, 0x77, 0x7,
    0xe0, 0xdc, 0x1f, 0x83, 0xf0, 0x7e, 0xf, 0x81,
    0xf0, 0x3e, 0x7, 0xc0, 0xf8, 0x1f, 0x3, 0xe0,
    0x7c, 0xf, 0x80,

    /* U+0058 "X" */
    0xf0, 0x0, 0xf7, 0x80, 0x1e, 0x78, 0x1, 0xe3,
    0xc0, 0x3c, 0x3e, 0x3, 0xc1, 0xe0, 0x78, 0xf,
    0x7, 0x0, 0xf0, 0xf0, 0x7, 0x8e, 0x0, 0x79,
    0xe0, 0x3, 0xdc, 0x0, 0x3f, 0xc0, 0x1, 0xf8,
    0x0, 0x1f, 0x80, 0x0, 0xf0, 0x0, 0xf, 0x80,
    0x1, 0xf8, 0x0, 0x3f, 0xc0, 0x3, 0xfc, 0x0,
    0x79, 0xe0, 0x7, 0x9e, 0x0, 0xf0, 0xf0, 0xf,
    0xf, 0x1, 0xe0, 0x78, 0x1e, 0x7, 0x83, 0xc0,
    0x3c, 0x38, 0x3, 0xc7, 0x80, 0x1e, 0xf0, 0x1,
    0xff, 0x0, 0xf,

    /* U+0059 "Y" */
    0xf0, 0x0, 0x3d, 0xe0, 0x1, 0xe7, 0x80, 0x7,
    0x8f, 0x0, 0x3c, 0x3c, 0x0, 0xf0, 0x78, 0x7,
    0x81, 0xe0, 0x1e, 0x3, 0xc0, 0x70, 0xf, 0x3,
    0xc0, 0x1e, 0xe, 0x0, 0x78, 0x78, 0x0, 0xf1,
    0xc0, 0x3, 0xcf, 0x0, 0x7, 0x38, 0x0, 0x1f,
    0xe0, 0x0, 0x7f, 0x80, 0x0, 0xfc, 0x0, 0x3,
    0xf0, 0x0, 0x7, 0x80, 0x0, 0x1e, 0x0, 0x0,
    0x78, 0x0, 0x1, 0xe0, 0x0, 0x7, 0x80, 0x0,
    0x1e, 0x0, 0x0, 0x78, 0x0, 0x1, 0xe0, 0x0,
    0x7, 0x80, 0x0, 0x1e, 0x0, 0x0, 0x78, 0x0,
    0x1, 0xe0, 0x0,

    /* U+005A "Z" */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x0,
    0xf0, 0x0, 0xf0, 0x0, 0x78, 0x0, 0x78, 0x0,
    0x3c, 0x0, 0x3c, 0x0, 0x1e, 0x0, 0x1e, 0x0,
    0xe, 0x0, 0xf, 0x0, 0xf, 0x0, 0x7, 0x80,
    0x7, 0x80, 0x3, 0xc0, 0x3, 0xc0, 0x1, 0xc0,
    0x1, 0xe0, 0x1, 0xe0, 0x0, 0xf0, 0x0, 0xf0,
    0x0, 0x78, 0x0, 0x78, 0x0, 0x3c, 0x0, 0x3c,
    0x0, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc,

    /* U+0061 "a" */
    0x3, 0xf8, 0x3, 0xff, 0x81, 0xff, 0xf0, 0xf8,
    0x3e, 0x3c, 0x7, 0x9e, 0x0, 0xf0, 0x0, 0x3c,
    0x0, 0xf, 0x0, 0x3, 0xc3, 0xff, 0xf3, 0xff,
    0xfd, 0xff, 0xff, 0x7c, 0x3, 0xfe, 0x0, 0xff,
    0x0, 0x3f, 0xc0, 0xf, 0xf0, 0x3, 0xfc, 0x1,
    0xff, 0x80, 0x7d, 0xf0, 0x7f, 0x7f, 0xfb, 0xcf,
    0xfc, 0xf0, 0xfe, 0x3c,

    /* U+0062 "b" */
    0xf0, 0x0, 0x78, 0x0, 0x3c, 0x0, 0x1e, 0x0,
    0xf, 0x0, 0x7, 0x80, 0x3, 0xc0, 0x1, 0xe3,
    0xf0, 0xf3, 0xfc, 0x7b, 0xff, 0x3f, 0x7, 0xdf,
    0x1, 0xef, 0x80, 0xff, 0x80, 0x3f, 0xc0, 0x1f,
    0xe0, 0xf, 0xf0, 0x7, 0xf8, 0x3, 0xfc, 0x1,
    0xfe, 0x0, 0xff, 0x0, 0x7f, 0x80, 0x3f, 0xc0,
    0x1f, 0xe0, 0xf, 0xf8, 0xf, 0xfc, 0x7, 0xbf,
    0x7, 0xde, 0xff, 0xcf, 0x7f, 0xc7, 0x8f, 0xc0,

    /* U+0063 "c" */
    0x7, 0xf0, 0xf, 0xfe, 0xf, 0xff, 0x8f, 0x83,
    0xe7, 0x80, 0xf7, 0xc0, 0x3f, 0xc0, 0x1f, 0xe0,
    0xf, 0xf0, 0x0, 0x78, 0x0, 0x3c, 0x0, 0x1e,
    0x0, 0xf, 0x0, 0x7, 0x80, 0x3, 0xc0, 0x1,
    0xe0, 0xf, 0xf0, 0x7, 0xfc, 0x3, 0xde, 0x3,
    0xcf, 0x83, 0xe3, 0xff, 0xe0, 0xff, 0xe0, 0x1f,
    0xc0,

    /* U+0064 "d" */
    0x0, 0x7, 0x80, 0x3, 0xc0, 0x1, 0xe0, 0x0,
    0xf0, 0x0, 0x78, 0x0, 0x3c, 0x0, 0x1e, 0x1f,
    0x8f, 0x1f, 0xe7, 0x9f, 0xfb, 0xdf, 0x7, 0xef,
    0x1, 0xff, 0x80, 0xff, 0x80, 0x3f, 0xc0, 0x1f,
    0xe0, 0xf, 0xf0, 0x7, 0xf8, 0x3, 0xfc, 0x1,
    0xfe, 0x0, 0xff, 0x0, 0x7f, 0x80, 0x3f, 0xc0,
    0x1f, 0xe0, 0xf, 0xf8, 0xf, 0xbc, 0x7, 0xdf,
    0x7, 0xe7, 0xfe, 0xf1, 0xfe, 0x78, 0x7e, 0x3c,

    /* U+0065 "e" */
    0x7, 0xf0, 0xf, 0xfe, 0xf, 0xff, 0x8f, 0x83,
    0xe7, 0x80, 0xf7, 0x80, 0x3f, 0xc0, 0x1f, 0xe0,
    0xf, 0xf0, 0x7, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0x0, 0x7, 0x80, 0x3, 0xc0, 0x1,
    0xe0, 0x0, 0xf0, 0x0, 0x7c, 0x3, 0xde, 0x3,
    0xcf, 0x83, 0xe3, 0xff, 0xe0, 0xff, 0xe0, 0x1f,
    0xc0,

    /* U+0066 "f" */
    0x0, 0x7f, 0xe0, 0x1f, 0xfc, 0x7, 0xff, 0x81,
    0xf0, 0x0, 0x3c, 0x0, 0x7, 0x80, 0x0, 0xf0,
    0x0, 0x1e, 0x0, 0x3, 0xc0, 0x0, 0x78, 0x3,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x7,
    0x80, 0x0, 0xf0, 0x0, 0x1e, 0x0, 0x3, 0xc0,
    0x0, 0x78, 0x0, 0xf, 0x0, 0x1, 0xe0, 0x0,
    0x3c, 0x0, 0x7, 0x80, 0x0, 0xf0, 0x0, 0x1e,
    0x0, 0x3, 0xc0, 0x0, 0x78, 0x0, 0xf, 0x0,
    0x1, 0xe0, 0x0, 0x3c, 0x0, 0x7, 0x80, 0x0,

    /* U+0067 "g" */
    0xf, 0xc7, 0x8f, 0xfb, 0xcf, 0xfd, 0xef, 0x83,
    0xf7, 0x80, 0xff, 0x80, 0x7f, 0xc0, 0x1f, 0xe0,
    0xf, 0xf0, 0x7, 0xf8, 0x3, 0xfc, 0x1, 0xfe,
    0x0, 0xff, 0x0, 0x7f, 0x80, 0x3f, 0xc0, 0x1f,
    0xe0, 0x1f, 0x78, 0xf, 0xbe, 0xf, 0xcf, 0xfd,
    0xe3, 0xfe, 0xf0, 0xfc, 0x78, 0x0, 0x3c, 0x0,
    0x1e, 0x0, 0xf, 0x0, 0x7, 0x80, 0x3, 0xc0,
    0x3, 0xc3, 0xff, 0xe1, 0xff, 0xe0, 0xff, 0xc0,

    /* U+0068 "h" */
    0xf0, 0x0, 0x78, 0x0, 0x3c, 0x0, 0x1e, 0x0,
    0xf, 0x0, 0x7, 0x80, 0x3, 0xc0, 0x1, 0xe3,
    0xf0, 0xf7, 0xfc, 0x7b, 0xff, 0x3f, 0x7, 0xdf,
    0x1, 0xef, 0x80, 0x7f, 0x80, 0x3f, 0xc0, 0x1f,
    0xe0, 0xf, 0xf0, 0x7, 0xf8, 0x3, 0xfc, 0x1,
    0xfe, 0x0, 0xff, 0x0, 0x7f, 0x80, 0x3f, 0xc0,
    0x1f, 0xe0, 0xf, 0xf0, 0x7, 0xf8, 0x3, 0xfc,
    0x1, 0xfe, 0x0, 0xff, 0x0, 0x7f, 0x80, 0x3c,

    /* U+0069 "i" */
    0x1, 0xe0, 0x0, 0x7e, 0x0, 0xf, 0xc0, 0x1,
    0xf8, 0x0, 0x1e, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0, 0x0, 0xf, 0xfe, 0x1,
    0xff, 0xc0, 0x3f, 0xf8, 0x0, 0xf, 0x0, 0x1,
    0xe0, 0x0, 0x3c, 0x0, 0x7, 0x80, 0x0, 0xf0,
    0x0, 0x1e, 0x0, 0x3, 0xc0, 0x0, 0x78, 0x0,
    0xf, 0x0, 0x1, 0xe0, 0x0, 0x3c, 0x0, 0x7,
    0x80, 0x0, 0xf0, 0x0, 0x1e, 0x0, 0x3, 0xc0,
    0x0, 0x78, 0x0, 0xf, 0x1, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff,

    /* U+006A "j" */
    0x0, 0x1e, 0x0, 0x3f, 0x0, 0x3f, 0x0, 0x3f,
    0x0, 0x1e, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x7f, 0xfe, 0x7f, 0xfe, 0x7f, 0xfe,
    0x0, 0x1e, 0x0, 0x1e, 0x0, 0x1e, 0x0, 0x1e,
    0x0, 0x1e, 0x0, 0x1e, 0x0, 0x1e, 0x0, 0x1e,
    0x0, 0x1e, 0x0, 0x1e, 0x0, 0x1e, 0x0, 0x1e,
    0x0, 0x1e, 0x0, 0x1e, 0x0, 0x1e, 0x0, 0x1e,
    0x0, 0x1e, 0x0, 0x1e, 0x0, 0x1e, 0x0, 0x1e,
    0x0, 0x1e, 0x0, 0x3e, 0x0, 0x3c, 0x0, 0xfc,
    0xff, 0xf8, 0xff, 0xf0, 0xff, 0x80,

    /* U+006B "k" */
    0xf0, 0x0, 0x1e, 0x0, 0x3, 0xc0, 0x0, 0x78,
    0x0, 0xf, 0x0, 0x1, 0xe0, 0x0, 0x3c, 0x0,
    0x7, 0x80, 0x1e, 0xf0, 0x3, 0xde, 0x0, 0xf3,
    0xc0, 0x3c, 0x78, 0x7, 0x8f, 0x1, 0xe1, 0xe0,
    0x7c, 0x3c, 0xf, 0x7, 0x83, 0xc0, 0xf0, 0x78,
    0x1f, 0xfe, 0x3, 0xff, 0x80, 0x7f, 0xf8, 0xf,
    0x7, 0x81, 0xe0, 0xf0, 0x3c, 0xf, 0x7, 0x80,
    0xf0, 0xf0, 0x1e, 0x1e, 0x1, 0xe3, 0xc0, 0x3e,
    0x78, 0x3, 0xcf, 0x0, 0x3d, 0xe0, 0x7, 0xc0,

    /* U+006C "l" */
    0xff, 0xf0, 0x7, 0xff, 0x80, 0x3f, 0xfc, 0x0,
    0x1, 0xe0, 0x0, 0xf, 0x0, 0x0, 0x78, 0x0,
    0x3, 0xc0, 0x0, 0x1e, 0x0, 0x0, 0xf0, 0x0,
    0x7, 0x80, 0x0, 0x3c, 0x0, 0x1, 0xe0, 0x0,
    0xf, 0x0, 0x0, 0x78, 0x0, 0x3, 0xc0, 0x0,
    0x1e, 0x0, 0x0, 0xf0, 0x0, 0x7, 0x80, 0x0,
    0x3c, 0x0, 0x1, 0xe0, 0x0, 0xf, 0x0, 0x0,
    0x78, 0x0, 0x3, 0xc0, 0x0, 0x1e, 0x0, 0x0,
    0xf0, 0x0, 0x7, 0x80, 0x0, 0x1e, 0x0, 0x0,
    0xff, 0xf0, 0x3, 0xff, 0x80, 0x7, 0xfc,

    /* U+006D "m" */
    0xe7, 0x87, 0x8e, 0xfc, 0xfe, 0xff, 0xdf, 0xef,
    0x1f, 0x1f, 0xe0, 0xe0, 0xfe, 0xe, 0xf, 0xe0,
    0xe0, 0xfe, 0xe, 0xf, 0xe0, 0xe0, 0xfe, 0xe,
    0xf, 0xe0, 0xe0, 0xfe, 0xe, 0xf, 0xe0, 0xe0,
    0xfe, 0xe, 0xf, 0xe0, 0xe0, 0xfe, 0xe, 0xf,
    0xe0, 0xe0, 0xfe, 0xe, 0xf, 0xe0, 0xe0, 0xfe,
    0xe, 0xf, 0xe0, 0xe0, 0xfe, 0xe, 0xf, 0xe0,
    0xe0, 0xf0,

    /* U+006E "n" */
    0xf1, 0xf8, 0x7b, 0xfe, 0x3d, 0xff, 0x9f, 0x83,
    0xef, 0x80, 0xf7, 0xc0, 0x3f, 0xc0, 0x1f, 0xe0,
    0xf, 0xf0, 0x7, 0xf8, 0x3, 0xfc, 0x1, 0xfe,
    0x0, 0xff, 0x0, 0x7f, 0x80, 0x3f, 0xc0, 0x1f,
    0xe0, 0xf, 0xf0, 0x7, 0xf8, 0x3, 0xfc, 0x1,
    0xfe, 0x0, 0xff, 0x0, 0x7f, 0x80, 0x3f, 0xc0,
    0x1e,

    /* U+006F "o" */
    0x7, 0xf0, 0xf, 0xfe, 0xf, 0xff, 0x8f, 0x83,
    0xe7, 0x80, 0xf7, 0xc0, 0x7f, 0xc0, 0x1f, 0xe0,
    0xf, 0xf0, 0x7, 0xf8, 0x3, 0xfc, 0x1, 0xfe,
    0x0, 0xff, 0x0, 0x7f, 0x80, 0x3f, 0xc0, 0x1f,
    0xe0, 0xf, 0xf0, 0x7, 0xfc, 0x7, 0xde, 0x3,
    0xcf, 0x83, 0xe3, 0xff, 0xe0, 0xff, 0xe0, 0x1f,
    0xc0,

    /* U+0070 "p" */
    0xf1, 0xf8, 0x79, 0xfe, 0x3d, 0xff, 0x9f, 0x83,
    0xef, 0x80, 0xf7, 0xc0, 0x7f, 0xc0, 0x1f, 0xe0,
    0xf, 0xf0, 0x7, 0xf8, 0x3, 0xfc, 0x1, 0xfe,
    0x0, 0xff, 0x0, 0x7f, 0x80, 0x3f, 0xc0, 0x1f,
    0xe0, 0xf, 0xf0, 0x7, 0xfc, 0x7, 0xfe, 0x3,
    0xdf, 0x83, 0xef, 0x7f, 0xe7, 0x9f, 0xe3, 0xc7,
    0xe1, 0xe0, 0x0, 0xf0, 0x0, 0x78, 0x0, 0x3c,
    0x0, 0x1e, 0x0, 0xf, 0x0, 0x7, 0x80, 0x0,

    /* U+0071 "q" */
    0xf, 0xc7, 0x8f, 0xfb, 0xcf, 0xfd, 0xef, 0x83,
    0xf7, 0x80, 0xff, 0xc0, 0x7f, 0xc0, 0x1f, 0xe0,
    0xf, 0xf0, 0x7, 0xf8, 0x3, 0xfc, 0x1, 0xfe,
    0x0, 0xff, 0x0, 0x7f, 0x80, 0x3f, 0xc0, 0x1f,
    0xe0, 0xf, 0xf0, 0x7, 0xf8, 0x7, 0xde, 0x3,
    0xef, 0x83, 0xf3, 0xff, 0x78, 0xff, 0x3c, 0x3f,
    0x1e, 0x0, 0xf, 0x0, 0x7, 0x80, 0x3, 0xc0,
    0x1, 0xe0, 0x0, 0xf0, 0x0, 0x78, 0x0, 0x3c,

    /* U+0072 "r" */
    0xf1, 0xf8, 0x7b, 0xfe, 0x3d, 0xff, 0x9f, 0x83,
    0xef, 0x80, 0xf7, 0x80, 0x7f, 0xc0, 0x1f, 0xe0,
    0xf, 0xf0, 0x7, 0xf8, 0x3, 0xfc, 0x0, 0x1e,
    0x0, 0xf, 0x0, 0x7, 0x80, 0x3, 0xc0, 0x1,
    0xe0, 0x0, 0xf0, 0x0, 0x78, 0x0, 0x3c, 0x0,
    0x1e, 0x0, 0xf, 0x0, 0x7, 0x80, 0x3, 0xc0,
    0x0,

    /* U+0073 "s" */
    0xf, 0xf8, 0x1f, 0xff, 0x1f, 0xff, 0xcf, 0x1,
    0xff, 0x0, 0x7f, 0x80, 0x3, 0xc0, 0x1, 0xe0,
    0x0, 0xf8, 0x0, 0x3f, 0xe0, 0x1f, 0xfe, 0x7,
    0xff, 0xc0, 0x7f, 0xf0, 0x1, 0xfc, 0x0, 0x3e,
    0x0, 0xf, 0x0, 0x7, 0xf8, 0x3, 0xfc, 0x1,
    0xff, 0x1, 0xe7, 0xff, 0xf1, 0xff, 0xf0, 0x3f,
    0xe0,

    /* U+0074 "t" */
    0x3, 0xc0, 0x0, 0x78, 0x0, 0xf, 0x0, 0x1,
    0xe0, 0x0, 0x3c, 0x0, 0x7, 0x80, 0x3f, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x78, 0x0,
    0xf, 0x0, 0x1, 0xe0, 0x0, 0x3c, 0x0, 0x7,
    0x80, 0x0, 0xf0, 0x0, 0x1e, 0x0, 0x3, 0xc0,
    0x0, 0x78, 0x0, 0xf, 0x0, 0x1, 0xe0, 0x0,
    0x3c, 0x0, 0x7, 0x80, 0x0, 0xf0, 0x0, 0x1e,
    0x0, 0x3, 0xc0, 0x0, 0x7c, 0x0, 0x7, 0xff,
    0x80, 0x7f, 0xf0, 0x7, 0xfe,

    /* U+0075 "u" */
    0xf0, 0x7, 0xf8, 0x3, 0xfc, 0x1, 0xfe, 0x0,
    0xff, 0x0, 0x7f, 0x80, 0x3f, 0xc0, 0x1f, 0xe0,
    0xf, 0xf0, 0x7, 0xf8, 0x3, 0xfc, 0x1, 0xfe,
    0x0, 0xff, 0x0, 0x7f, 0x80, 0x3f, 0xc0, 0x1f,
    0xe0, 0xf, 0xf0, 0x7, 0xfc, 0x7, 0xde, 0x3,
    0xcf, 0x83, 0xe3, 0xff, 0xe0, 0xff, 0xe0, 0x1f,
    0xc0,

    /* U+0076 "v" */
    0xf0, 0x0, 0xff, 0x0, 0xf, 0x78, 0x1, 0xe7,
    0x80, 0x1e, 0x78, 0x1, 0xe3, 0xc0, 0x3c, 0x3c,
    0x3, 0xc3, 0xc0, 0x3c, 0x1e, 0x7, 0x81, 0xe0,
    0x78, 0x1e, 0x7, 0x80, 0xf0, 0x70, 0xf, 0xf,
    0x0, 0xf0, 0xf0, 0x7, 0x8e, 0x0, 0x79, 0xe0,
    0x7, 0x9e, 0x0, 0x39, 0xc0, 0x3, 0xfc, 0x0,
    0x3f, 0xc0, 0x1, 0xf8, 0x0, 0x1f, 0x80, 0x1,
    0xf8, 0x0,

    /* U+0077 "w" */
    0xf0, 0x78, 0x3b, 0x83, 0xc1, 0xdc, 0x1e, 0xe,
    0xe0, 0xf0, 0x77, 0xf, 0x83, 0xb8, 0x6c, 0x1d,
    0xc3, 0x70, 0xce, 0x19, 0x8e, 0x38, 0xcc, 0x71,
    0xc6, 0x63, 0x8e, 0x33, 0x1c, 0x73, 0x98, 0xe3,
    0x98, 0xc6, 0x1c, 0xc7, 0x30, 0x66, 0x1b, 0x83,
    0xb0, 0xdc, 0x1d, 0x86, 0xe0, 0xfc, 0x37, 0x7,
    0xc1, 0xb0, 0x3e, 0xd, 0x81, 0xf0, 0x7c, 0x7,
    0x81, 0xe0, 0x3c, 0xf, 0x0,

    /* U+0078 "x" */
    0x78, 0x1, 0xe7, 0xc0, 0x3e, 0x3c, 0x3, 0xc1,
    0xe0, 0x78, 0x1e, 0x7, 0x80, 0xf0, 0xf0, 0x7,
    0x9e, 0x0, 0x79, 0xe0, 0x3, 0xfc, 0x0, 0x1f,
    0x80, 0x1, 0xf8, 0x0, 0xf, 0x0, 0x1, 0xf8,
    0x0, 0x3f, 0xc0, 0x3, 0xdc, 0x0, 0x79, 0xe0,
    0xf, 0xf, 0x0, 0xf0, 0xf0, 0x1e, 0x7, 0x83,
    0xc0, 0x3c, 0x3c, 0x3, 0xc7, 0x80, 0x1e, 0xf8,
    0x1, 0xf0,

    /* U+0079 "y" */
    0xf0, 0x0, 0xf7, 0x0, 0xe, 0x78, 0x1, 0xe7,
    0x80, 0x1e, 0x3c, 0x1, 0xc3, 0xc0, 0x3c, 0x1c,
    0x3, 0xc1, 0xe0, 0x38, 0x1e, 0x7, 0x80, 0xf0,
    0x78, 0xf, 0xf, 0x0, 0x70, 0xf0, 0x7, 0x8e,
    0x0, 0x79, 0xe0, 0x3, 0x9e, 0x0, 0x3d, 0xc0,
    0x1, 0xfc, 0x0, 0x1f, 0xc0, 0x1, 0xf8, 0x0,
    0xf, 0x80, 0x0, 0xf0, 0x0, 0xf, 0x0, 0x0,
    0xf0, 0x0, 0xe, 0x0, 0x1, 0xe0, 0x0, 0x1e,
    0x0, 0x3, 0xc0, 0x0, 0x3c, 0x0, 0x3, 0x80,
    0x0, 0x78, 0x0,

    /* U+007A "z" */
    0xff, 0xff, 0x7f, 0xff, 0xbf, 0xff, 0xc0, 0x1,
    0xe0, 0x1, 0xe0, 0x1, 0xe0, 0x1, 0xf0, 0x0,
    0xf0, 0x0, 0xf0, 0x0, 0xf0, 0x0, 0x78, 0x0,
    0x78, 0x0, 0x78, 0x0, 0x78, 0x0, 0x3c, 0x0,
    0x3c, 0x0, 0x3c, 0x0, 0x1c, 0x0, 0x1e, 0x0,
    0x1e, 0x0, 0xf, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xfe
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 384, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 384, .box_w = 13, .box_h = 3, .ofs_x = 6, .ofs_y = 12},
    {.bitmap_index = 6, .adv_w = 384, .box_w = 18, .box_h = 30, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 74, .adv_w = 384, .box_w = 18, .box_h = 30, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 142, .adv_w = 384, .box_w = 18, .box_h = 30, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 210, .adv_w = 384, .box_w = 18, .box_h = 30, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 278, .adv_w = 384, .box_w = 17, .box_h = 30, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 342, .adv_w = 384, .box_w = 18, .box_h = 30, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 410, .adv_w = 384, .box_w = 19, .box_h = 30, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 482, .adv_w = 384, .box_w = 19, .box_h = 30, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 554, .adv_w = 384, .box_w = 19, .box_h = 30, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 626, .adv_w = 384, .box_w = 19, .box_h = 30, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 698, .adv_w = 384, .box_w = 20, .box_h = 30, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 773, .adv_w = 384, .box_w = 18, .box_h = 30, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 841, .adv_w = 384, .box_w = 17, .box_h = 30, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 905, .adv_w = 384, .box_w = 17, .box_h = 30, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 969, .adv_w = 384, .box_w = 17, .box_h = 30, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 1033, .adv_w = 384, .box_w = 17, .box_h = 30, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 1097, .adv_w = 384, .box_w = 17, .box_h = 30, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 1161, .adv_w = 384, .box_w = 17, .box_h = 30, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 1225, .adv_w = 384, .box_w = 16, .box_h = 30, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 1285, .adv_w = 384, .box_w = 18, .box_h = 30, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1353, .adv_w = 384, .box_w = 19, .box_h = 30, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 1425, .adv_w = 384, .box_w = 17, .box_h = 30, .ofs_x = 5, .ofs_y = 0},
    {.bitmap_index = 1489, .adv_w = 384, .box_w = 18, .box_h = 30, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 1557, .adv_w = 384, .box_w = 17, .box_h = 30, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 1621, .adv_w = 384, .box_w = 17, .box_h = 30, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 1685, .adv_w = 384, .box_w = 18, .box_h = 30, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 1753, .adv_w = 384, .box_w = 18, .box_h = 37, .ofs_x = 3, .ofs_y = -7},
    {.bitmap_index = 1837, .adv_w = 384, .box_w = 18, .box_h = 30, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 1905, .adv_w = 384, .box_w = 18, .box_h = 30, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 1973, .adv_w = 384, .box_w = 20, .box_h = 30, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 2048, .adv_w = 384, .box_w = 17, .box_h = 30, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 2112, .adv_w = 384, .box_w = 20, .box_h = 30, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 2187, .adv_w = 384, .box_w = 22, .box_h = 30, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 2270, .adv_w = 384, .box_w = 20, .box_h = 30, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 2345, .adv_w = 384, .box_w = 22, .box_h = 30, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 2428, .adv_w = 384, .box_w = 17, .box_h = 30, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 2492, .adv_w = 384, .box_w = 18, .box_h = 23, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 2544, .adv_w = 384, .box_w = 17, .box_h = 30, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 2608, .adv_w = 384, .box_w = 17, .box_h = 23, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 2657, .adv_w = 384, .box_w = 17, .box_h = 30, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 2721, .adv_w = 384, .box_w = 17, .box_h = 23, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 2770, .adv_w = 384, .box_w = 19, .box_h = 30, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 2842, .adv_w = 384, .box_w = 17, .box_h = 30, .ofs_x = 4, .ofs_y = -7},
    {.bitmap_index = 2906, .adv_w = 384, .box_w = 17, .box_h = 30, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 2970, .adv_w = 384, .box_w = 19, .box_h = 32, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 3046, .adv_w = 384, .box_w = 16, .box_h = 39, .ofs_x = 3, .ofs_y = -7},
    {.bitmap_index = 3124, .adv_w = 384, .box_w = 19, .box_h = 30, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 3196, .adv_w = 384, .box_w = 21, .box_h = 30, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 3275, .adv_w = 384, .box_w = 20, .box_h = 23, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 3333, .adv_w = 384, .box_w = 17, .box_h = 23, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 3382, .adv_w = 384, .box_w = 17, .box_h = 23, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 3431, .adv_w = 384, .box_w = 17, .box_h = 30, .ofs_x = 4, .ofs_y = -7},
    {.bitmap_index = 3495, .adv_w = 384, .box_w = 17, .box_h = 30, .ofs_x = 4, .ofs_y = -7},
    {.bitmap_index = 3559, .adv_w = 384, .box_w = 17, .box_h = 23, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 3608, .adv_w = 384, .box_w = 17, .box_h = 23, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 3657, .adv_w = 384, .box_w = 19, .box_h = 29, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 3726, .adv_w = 384, .box_w = 17, .box_h = 23, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 3775, .adv_w = 384, .box_w = 20, .box_h = 23, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 3833, .adv_w = 384, .box_w = 21, .box_h = 23, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 3894, .adv_w = 384, .box_w = 20, .box_h = 23, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 3952, .adv_w = 384, .box_w = 20, .box_h = 30, .ofs_x = 2, .ofs_y = -7},
    {.bitmap_index = 4027, .adv_w = 384, .box_w = 17, .box_h = 23, .ofs_x = 4, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {
    0x0, 0xd
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 14, .glyph_id_start = 1,
        .unicode_list = unicode_list_0, .glyph_id_ofs_list = NULL, .list_length = 2, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    },
    {
        .range_start = 48, .range_length = 10, .glyph_id_start = 3,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 65, .range_length = 26, .glyph_id_start = 13,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 97, .range_length = 26, .glyph_id_start = 39,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LV_VERSION_CHECK(8, 0, 0)
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 4,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LV_VERSION_CHECK(8, 0, 0)
    .cache = &cache
#endif
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LV_VERSION_CHECK(8, 0, 0)
const lv_font_t jetbrains_mono_40 = {
#else
lv_font_t jetbrains_mono_40 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 39,          /*The maximum line height required by the font*/
    .base_line = 7,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0)
    .underline_position = -6,
    .underline_thickness = 2,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if JETBRAINS_MONO_40*/

