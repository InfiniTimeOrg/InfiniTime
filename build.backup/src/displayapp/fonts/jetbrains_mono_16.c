/*******************************************************************************
 * Size: 16 px
 * Bpp: 1
 * Opts: --size 16 --output jetbrains_mono_16.c --bpp 1 --format lvgl --no-compress --font /sources/src/displayapp/fonts/JetBrainsMono-Regular.ttf --range 0x20-0x7e, 0x410-0x44f, 0xB0
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef JETBRAINS_MONO_16
#define JETBRAINS_MONO_16 1
#endif

#if JETBRAINS_MONO_16

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */
    0x0,

    /* U+0021 "!" */
    0x55, 0x55, 0xf,

    /* U+0022 "\"" */
    0x99, 0x99, 0x90,

    /* U+0023 "#" */
    0x11, 0x8, 0x84, 0x82, 0x47, 0xf9, 0x10, 0x88,
    0x44, 0xff, 0x12, 0x19, 0x8, 0x80,

    /* U+0024 "$" */
    0x10, 0x20, 0x41, 0xe5, 0x6a, 0x54, 0x38, 0x38,
    0x38, 0x4c, 0x99, 0x2f, 0x8e, 0x8, 0x10,

    /* U+0025 "%" */
    0x60, 0xc8, 0xa4, 0x92, 0x49, 0x43, 0x40, 0x20,
    0x26, 0x24, 0x92, 0x51, 0x30, 0x60,

    /* U+0026 "&" */
    0x38, 0x44, 0x44, 0x40, 0x60, 0x60, 0x92, 0x9e,
    0x8c, 0x84, 0x8e, 0x72,

    /* U+0027 "'" */
    0xf8,

    /* U+0028 "(" */
    0x0, 0xc8, 0x88, 0x42, 0x10, 0x84, 0x21, 0x4,
    0x30, 0xe1,

    /* U+0029 ")" */
    0x87, 0xc, 0x20, 0x84, 0x21, 0x8, 0x42, 0x11,
    0x1b, 0x98,

    /* U+002A "*" */
    0x8, 0x4, 0x12, 0x4f, 0xe1, 0x80, 0xa0, 0xd8,
    0x44,

    /* U+002B "+" */
    0x10, 0x10, 0x10, 0xff, 0x10, 0x10, 0x10,

    /* U+002C "," */
    0x6b, 0x68,

    /* U+002D "-" */
    0xf8,

    /* U+002E "." */
    0xfc,

    /* U+002F "/" */
    0x2, 0xc, 0x10, 0x20, 0xc1, 0x6, 0xc, 0x10,
    0x60, 0x81, 0x6, 0x8, 0x30, 0x0,

    /* U+0030 "0" */
    0x7d, 0x8e, 0xc, 0x18, 0x33, 0x66, 0xc1, 0x83,
    0x7, 0x13, 0xc0,

    /* U+0031 "1" */
    0x30, 0xe3, 0x40, 0x81, 0x2, 0x4, 0x8, 0x10,
    0x20, 0x47, 0xf0,

    /* U+0032 "2" */
    0x79, 0x8a, 0x8, 0x10, 0x20, 0x83, 0xc, 0x30,
    0xc3, 0x7, 0xf0,

    /* U+0033 "3" */
    0x7e, 0xc, 0x30, 0xc3, 0x87, 0x80, 0x81, 0x3,
    0x7, 0x11, 0xc0,

    /* U+0034 "4" */
    0xc, 0x10, 0x61, 0x82, 0x8, 0x70, 0xc1, 0xfe,
    0x4, 0x8, 0x10,

    /* U+0035 "5" */
    0xfe, 0x8, 0x20, 0xfb, 0x30, 0x41, 0x6, 0x18,
    0xde,

    /* U+0036 "6" */
    0x18, 0x20, 0xc1, 0x6, 0xf, 0xb1, 0xc1, 0x83,
    0x5, 0x11, 0xc0,

    /* U+0037 "7" */
    0xff, 0x82, 0x86, 0x6, 0xc, 0xc, 0x8, 0x18,
    0x18, 0x30, 0x30, 0x20,

    /* U+0038 "8" */
    0x3d, 0x8e, 0xc, 0x1c, 0x6f, 0x9b, 0x63, 0x83,
    0x7, 0x1b, 0xe0,

    /* U+0039 "9" */
    0x38, 0x8a, 0xc, 0x18, 0x38, 0xdf, 0x6, 0x8,
    0x30, 0x41, 0x80,

    /* U+003A ":" */
    0xfc, 0x0, 0x7, 0xe0,

    /* U+003B ";" */
    0x77, 0x0, 0x0, 0x7, 0x66, 0x4c,

    /* U+003C "<" */
    0x0, 0xc, 0x73, 0x88, 0x1c, 0xe, 0x7, 0x2,

    /* U+003D "=" */
    0xfe, 0x0, 0x0, 0xf, 0xe0,

    /* U+003E ">" */
    0x1, 0x81, 0xc0, 0xe0, 0x61, 0xce, 0x70, 0x80,

    /* U+003F "?" */
    0xf0, 0x20, 0x41, 0x4, 0x33, 0x88, 0x20, 0x6,
    0x18,

    /* U+0040 "@" */
    0x3c, 0x42, 0xc1, 0x81, 0x8f, 0x91, 0x91, 0x91,
    0x91, 0x91, 0x8f, 0x80, 0x80, 0x40, 0x38,

    /* U+0041 "A" */
    0x18, 0x38, 0x38, 0x2c, 0x2c, 0x64, 0x64, 0x46,
    0x7e, 0xc2, 0xc2, 0x83,

    /* U+0042 "B" */
    0xfd, 0xe, 0xc, 0x18, 0x7f, 0xa1, 0xc1, 0x83,
    0x6, 0x1f, 0xe0,

    /* U+0043 "C" */
    0x7d, 0x8e, 0xc, 0x8, 0x10, 0x20, 0x40, 0x81,
    0x7, 0x1b, 0xe0,

    /* U+0044 "D" */
    0xf9, 0xa, 0xc, 0x18, 0x30, 0x60, 0xc1, 0x83,
    0x6, 0x17, 0xc0,

    /* U+0045 "E" */
    0xfe, 0x8, 0x20, 0x83, 0xf8, 0x20, 0x82, 0x8,
    0x3f,

    /* U+0046 "F" */
    0xfe, 0x8, 0x20, 0x82, 0xf, 0xe0, 0x82, 0x8,
    0x20,

    /* U+0047 "G" */
    0x7d, 0x8e, 0xc, 0x8, 0x10, 0x27, 0xc1, 0x83,
    0x7, 0x1b, 0xe0,

    /* U+0048 "H" */
    0x86, 0x18, 0x61, 0x87, 0xf8, 0x61, 0x86, 0x18,
    0x61,

    /* U+0049 "I" */
    0xfc, 0x41, 0x4, 0x10, 0x41, 0x4, 0x10, 0x41,
    0x3f,

    /* U+004A "J" */
    0x3e, 0x4, 0x8, 0x10, 0x20, 0x40, 0x81, 0x3,
    0x7, 0x13, 0xc0,

    /* U+004B "K" */
    0x87, 0xa, 0x34, 0x49, 0x9e, 0x24, 0x4c, 0x89,
    0x1a, 0x14, 0x30,

    /* U+004C "L" */
    0x82, 0x8, 0x20, 0x82, 0x8, 0x20, 0x82, 0x8,
    0x3f,

    /* U+004D "M" */
    0xc7, 0x8f, 0x1d, 0x5a, 0xb5, 0x64, 0xc1, 0x83,
    0x6, 0xc, 0x10,

    /* U+004E "N" */
    0xc3, 0x87, 0x8d, 0x1b, 0x32, 0x64, 0xcd, 0x8b,
    0x1e, 0x1c, 0x30,

    /* U+004F "O" */
    0x7d, 0x8e, 0xc, 0x18, 0x30, 0x60, 0xc1, 0x83,
    0x7, 0x1b, 0xe0,

    /* U+0050 "P" */
    0xfd, 0xa, 0xc, 0x18, 0x30, 0xbe, 0x40, 0x81,
    0x2, 0x4, 0x0,

    /* U+0051 "Q" */
    0x79, 0x8a, 0xc, 0x18, 0x30, 0x60, 0xc1, 0x83,
    0x5, 0x19, 0xe0, 0xc0, 0x80, 0x80,

    /* U+0052 "R" */
    0xf9, 0xa, 0xc, 0x18, 0x30, 0xff, 0x44, 0x8d,
    0xa, 0x1c, 0x10,

    /* U+0053 "S" */
    0x3c, 0xcd, 0xa, 0x6, 0x7, 0x7, 0x3, 0x3,
    0x7, 0x1b, 0xe0,

    /* U+0054 "T" */
    0xff, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10,

    /* U+0055 "U" */
    0x83, 0x6, 0xc, 0x18, 0x30, 0x60, 0xc1, 0x83,
    0x7, 0x1b, 0xe0,

    /* U+0056 "V" */
    0x83, 0xc2, 0xc2, 0x46, 0x46, 0x64, 0x64, 0x24,
    0x2c, 0x38, 0x38, 0x18,

    /* U+0057 "W" */
    0x99, 0x99, 0x99, 0x99, 0x99, 0xdb, 0xe7, 0xe6,
    0x66, 0x66, 0x66, 0x66,

    /* U+0058 "X" */
    0xc3, 0x46, 0x64, 0x2c, 0x38, 0x18, 0x18, 0x3c,
    0x2c, 0x66, 0xc2, 0xc3,

    /* U+0059 "Y" */
    0x41, 0x31, 0x88, 0x84, 0x41, 0x40, 0xa0, 0x20,
    0x10, 0x8, 0x4, 0x2, 0x1, 0x0,

    /* U+005A "Z" */
    0xfe, 0xc, 0x10, 0x60, 0x83, 0xc, 0x10, 0x60,
    0x83, 0x7, 0xf0,

    /* U+005B "[" */
    0xf2, 0x49, 0x24, 0x92, 0x49, 0x38,

    /* U+005C "\\" */
    0xc0, 0x81, 0x81, 0x2, 0x6, 0x4, 0xc, 0x18,
    0x10, 0x30, 0x20, 0x40, 0xc0, 0x80,

    /* U+005D "]" */
    0xe4, 0x92, 0x49, 0x24, 0x92, 0x78,

    /* U+005E "^" */
    0x10, 0x30, 0xe1, 0x26, 0x48, 0xf0, 0x80,

    /* U+005F "_" */
    0xff,

    /* U+0060 "`" */
    0x99, 0x80,

    /* U+0061 "a" */
    0x3c, 0x8c, 0x8, 0x17, 0xf0, 0x60, 0xc3, 0x7a,

    /* U+0062 "b" */
    0x81, 0x2, 0x5, 0xec, 0x70, 0x60, 0xc1, 0x83,
    0x7, 0x1d, 0xe0,

    /* U+0063 "c" */
    0x3d, 0x8e, 0xc, 0x8, 0x10, 0x20, 0xe3, 0x7c,

    /* U+0064 "d" */
    0x2, 0x4, 0xb, 0xdc, 0x70, 0x60, 0xc1, 0x83,
    0x7, 0x1b, 0xd0,

    /* U+0065 "e" */
    0x79, 0x8a, 0xc, 0x1f, 0xf0, 0x20, 0x63, 0x7c,

    /* U+0066 "f" */
    0xf, 0x10, 0x10, 0x10, 0xff, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10,

    /* U+0067 "g" */
    0x77, 0x38, 0x61, 0x86, 0x1c, 0xdd, 0x4, 0x10,
    0x5e,

    /* U+0068 "h" */
    0x82, 0x8, 0x2e, 0xce, 0x18, 0x61, 0x86, 0x18,
    0x61,

    /* U+0069 "i" */
    0x18, 0x18, 0x0, 0x0, 0xf0, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0xff,

    /* U+006A "j" */
    0xc, 0x30, 0x0, 0xfc, 0x10, 0x41, 0x4, 0x10,
    0x41, 0x4, 0x10, 0xbc,

    /* U+006B "k" */
    0x81, 0x2, 0x4, 0x38, 0xd1, 0x26, 0x78, 0x99,
    0x12, 0x34, 0x30,

    /* U+006C "l" */
    0xf0, 0x8, 0x4, 0x2, 0x1, 0x0, 0x80, 0x40,
    0x20, 0x10, 0x8, 0x4, 0x1, 0xf0,

    /* U+006D "m" */
    0xfe, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91,
    0x91,

    /* U+006E "n" */
    0xbb, 0x38, 0x61, 0x86, 0x18, 0x61, 0x84,

    /* U+006F "o" */
    0x7d, 0x8e, 0xc, 0x18, 0x30, 0x60, 0xe3, 0x7c,

    /* U+0070 "p" */
    0xbd, 0x8e, 0xc, 0x18, 0x30, 0x60, 0xe3, 0xbd,
    0x2, 0x4, 0x0,

    /* U+0071 "q" */
    0x7b, 0x8e, 0xc, 0x18, 0x30, 0x60, 0xe3, 0x7a,
    0x4, 0x8, 0x10,

    /* U+0072 "r" */
    0xbd, 0x8e, 0xc, 0x18, 0x10, 0x20, 0x40, 0x80,

    /* U+0073 "s" */
    0x7d, 0x6, 0x7, 0x7, 0xc0, 0xc0, 0xc1, 0x7c,

    /* U+0074 "t" */
    0x10, 0x20, 0x47, 0xf1, 0x2, 0x4, 0x8, 0x10,
    0x20, 0x40, 0x70,

    /* U+0075 "u" */
    0x83, 0x6, 0xc, 0x18, 0x30, 0x60, 0xe3, 0x7c,

    /* U+0076 "v" */
    0x83, 0xc2, 0x46, 0x46, 0x64, 0x2c, 0x2c, 0x38,
    0x18,

    /* U+0077 "w" */
    0x99, 0x99, 0x9b, 0xaa, 0xaa, 0xea, 0x66, 0x66,
    0x66,

    /* U+0078 "x" */
    0xc2, 0x66, 0x2c, 0x38, 0x18, 0x38, 0x6c, 0x46,
    0xc2,

    /* U+0079 "y" */
    0xc3, 0xc2, 0x46, 0x66, 0x24, 0x2c, 0x38, 0x18,
    0x18, 0x10, 0x30, 0x30,

    /* U+007A "z" */
    0xfe, 0xc, 0x30, 0xc1, 0x6, 0x18, 0x60, 0xfe,

    /* U+007B "{" */
    0xe, 0x30, 0x60, 0xc1, 0x83, 0x6, 0x70, 0x18,
    0x30, 0x60, 0xc1, 0x83, 0x3, 0x80,

    /* U+007C "|" */
    0xff, 0xfe,

    /* U+007D "}" */
    0xe0, 0x60, 0xc1, 0x83, 0x4, 0xc, 0xf, 0x30,
    0x40, 0xc1, 0x83, 0x6, 0x38, 0x0,

    /* U+007E "~" */
    0x73, 0x26, 0x70,

    /* U+00B0 "°" */
    0x74, 0x63, 0x17, 0x0,

    /* U+0410 "А" */
    0x18, 0x38, 0x38, 0x2c, 0x2c, 0x64, 0x64, 0x46,
    0x7e, 0xc2, 0xc2, 0x83,

    /* U+0411 "Б" */
    0xff, 0x2, 0x4, 0x8, 0x1f, 0x21, 0x41, 0x83,
    0x6, 0x17, 0xc0,

    /* U+0412 "В" */
    0xfd, 0xe, 0xc, 0x18, 0x7f, 0xa1, 0xc1, 0x83,
    0x6, 0x1f, 0xe0,

    /* U+0413 "Г" */
    0xfe, 0x8, 0x20, 0x82, 0x8, 0x20, 0x82, 0x8,
    0x20,

    /* U+0414 "Д" */
    0x3e, 0x11, 0x8, 0x84, 0x42, 0x21, 0x10, 0x88,
    0x44, 0x22, 0x31, 0x18, 0x9f, 0xf8, 0xc, 0x4,

    /* U+0415 "Е" */
    0xfe, 0x8, 0x20, 0x83, 0xf8, 0x20, 0x82, 0x8,
    0x3f,

    /* U+0416 "Ж" */
    0xc9, 0xa4, 0x9a, 0x4d, 0x62, 0xa1, 0xf0, 0xa8,
    0xd6, 0x6b, 0x24, 0xb2, 0x79, 0x30,

    /* U+0417 "З" */
    0x7d, 0x8e, 0x8, 0x10, 0x67, 0x81, 0x81, 0x3,
    0x7, 0x11, 0xc0,

    /* U+0418 "И" */
    0x87, 0xe, 0x3c, 0x59, 0xb2, 0x64, 0xd9, 0xa3,
    0xc7, 0xe, 0x10,

    /* U+0419 "Й" */
    0x44, 0x88, 0xe0, 0x8, 0x71, 0xe3, 0xc5, 0x9b,
    0x36, 0xcd, 0x9a, 0x3c, 0x78, 0xe1,

    /* U+041A "К" */
    0x87, 0xa, 0x34, 0x49, 0x9e, 0x24, 0x4c, 0x89,
    0x1a, 0x14, 0x30,

    /* U+041B "Л" */
    0x3f, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21,
    0x21, 0x21, 0x21, 0xc1,

    /* U+041C "М" */
    0xc7, 0x8f, 0x1d, 0x5a, 0xb5, 0x64, 0xc1, 0x83,
    0x6, 0xc, 0x10,

    /* U+041D "Н" */
    0x86, 0x18, 0x61, 0x87, 0xf8, 0x61, 0x86, 0x18,
    0x61,

    /* U+041E "О" */
    0x7d, 0x8e, 0xc, 0x18, 0x30, 0x60, 0xc1, 0x83,
    0x7, 0x1b, 0xe0,

    /* U+041F "П" */
    0xfe, 0x18, 0x61, 0x86, 0x18, 0x61, 0x86, 0x18,
    0x61,

    /* U+0420 "Р" */
    0xfd, 0xa, 0xc, 0x18, 0x30, 0xbe, 0x40, 0x81,
    0x2, 0x4, 0x0,

    /* U+0421 "С" */
    0x7d, 0x8e, 0xc, 0x8, 0x10, 0x20, 0x40, 0x81,
    0x7, 0x1b, 0xe0,

    /* U+0422 "Т" */
    0xff, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10,

    /* U+0423 "У" */
    0xc3, 0xc2, 0x46, 0x66, 0x24, 0x2c, 0x3c, 0x18,
    0x18, 0x10, 0x10, 0x30,

    /* U+0424 "Ф" */
    0x10, 0xfb, 0x5c, 0x99, 0x32, 0x64, 0xc9, 0x93,
    0x27, 0x5b, 0xe1, 0x0,

    /* U+0425 "Х" */
    0xc3, 0x46, 0x64, 0x2c, 0x38, 0x18, 0x18, 0x3c,
    0x2c, 0x66, 0xc2, 0xc3,

    /* U+0426 "Ц" */
    0x85, 0xa, 0x14, 0x28, 0x50, 0xa1, 0x42, 0x85,
    0xa, 0x17, 0xf0, 0x20, 0x40,

    /* U+0427 "Ч" */
    0x83, 0x6, 0xc, 0x18, 0x38, 0x5f, 0x81, 0x2,
    0x4, 0x8, 0x10,

    /* U+0428 "Ш" */
    0x93, 0x26, 0x4c, 0x99, 0x32, 0x64, 0xc9, 0x93,
    0x26, 0x4f, 0xf0,

    /* U+0429 "Щ" */
    0x92, 0x92, 0x92, 0x92, 0x92, 0x92, 0x92, 0x92,
    0x92, 0x92, 0x92, 0xff, 0x1, 0x1,

    /* U+042A "Ъ" */
    0xe0, 0x20, 0x20, 0x20, 0x20, 0x3c, 0x22, 0x21,
    0x21, 0x21, 0x22, 0x3e,

    /* U+042B "Ы" */
    0x83, 0x6, 0xc, 0x18, 0x3e, 0x66, 0xc5, 0x8b,
    0x16, 0x6f, 0x90,

    /* U+042C "Ь" */
    0x81, 0x2, 0x4, 0x8, 0x1f, 0x21, 0x41, 0x83,
    0x6, 0x17, 0xc0,

    /* U+042D "Э" */
    0x7d, 0x8e, 0x8, 0x10, 0x2f, 0xc0, 0x81, 0x83,
    0x7, 0x1b, 0xe0,

    /* U+042E "Ю" */
    0x8e, 0x91, 0x91, 0x91, 0x91, 0xf1, 0x91, 0x91,
    0x91, 0x91, 0x91, 0x8e,

    /* U+042F "Я" */
    0x3e, 0x86, 0xc, 0x18, 0x38, 0x5f, 0x91, 0x62,
    0x87, 0xc, 0x10,

    /* U+0430 "а" */
    0x3c, 0x8c, 0x8, 0x17, 0xf0, 0x60, 0xc3, 0x7a,

    /* U+0431 "б" */
    0x3e, 0x82, 0x4, 0xb, 0xd8, 0xe0, 0xc1, 0x83,
    0x7, 0x1b, 0xe0,

    /* U+0432 "в" */
    0xf9, 0xa, 0x14, 0x2f, 0x90, 0xe0, 0xc1, 0xfc,

    /* U+0433 "г" */
    0xfe, 0x8, 0x20, 0x82, 0x8, 0x20, 0x80,

    /* U+0434 "д" */
    0x3e, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x62,
    0xff, 0x81, 0x81,

    /* U+0435 "е" */
    0x79, 0x8a, 0xc, 0x1f, 0xf0, 0x20, 0x63, 0x7c,

    /* U+0436 "ж" */
    0xc9, 0xa4, 0x9a, 0xc5, 0x43, 0xe1, 0x51, 0xac,
    0x92, 0xc9, 0x80,

    /* U+0437 "з" */
    0x3c, 0x42, 0x2, 0x2, 0x3c, 0x3, 0x1, 0x41,
    0x3e,

    /* U+0438 "и" */
    0x8e, 0x39, 0xe5, 0xb6, 0x9e, 0x71, 0xc4,

    /* U+0439 "й" */
    0x45, 0x13, 0x80, 0x86, 0x38, 0xe5, 0x96, 0x9a,
    0x79, 0xc4,

    /* U+043A "к" */
    0x87, 0x1a, 0x24, 0xcf, 0x13, 0x22, 0x46, 0x86,

    /* U+043B "л" */
    0x3f, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21, 0x21,
    0xc1,

    /* U+043C "м" */
    0xc7, 0x8e, 0xad, 0x5a, 0xb7, 0x60, 0xc1, 0x82,

    /* U+043D "н" */
    0x86, 0x18, 0x61, 0xfe, 0x18, 0x61, 0x84,

    /* U+043E "о" */
    0x7d, 0x8e, 0xc, 0x18, 0x30, 0x60, 0xe3, 0x7c,

    /* U+043F "п" */
    0xfe, 0x18, 0x61, 0x86, 0x18, 0x61, 0x84,

    /* U+0440 "р" */
    0xbd, 0x8e, 0xc, 0x18, 0x30, 0x60, 0xe3, 0xbd,
    0x2, 0x4, 0x0,

    /* U+0441 "с" */
    0x3d, 0x8e, 0xc, 0x8, 0x10, 0x20, 0xe3, 0x7c,

    /* U+0442 "т" */
    0xff, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10,

    /* U+0443 "у" */
    0xc3, 0xc2, 0x46, 0x66, 0x24, 0x2c, 0x38, 0x18,
    0x18, 0x10, 0x30, 0x30,

    /* U+0444 "ф" */
    0x10, 0x20, 0x43, 0xed, 0x72, 0x64, 0xc9, 0x93,
    0x27, 0x5b, 0xe1, 0x2, 0x4, 0x0,

    /* U+0445 "х" */
    0xc2, 0x66, 0x2c, 0x38, 0x18, 0x38, 0x6c, 0x46,
    0xc2,

    /* U+0446 "ц" */
    0x85, 0xa, 0x14, 0x28, 0x50, 0xa1, 0x42, 0xfe,
    0x4, 0x8,

    /* U+0447 "ч" */
    0x83, 0x6, 0xc, 0x18, 0x2f, 0xc0, 0x81, 0x2,

    /* U+0448 "ш" */
    0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91, 0x91,
    0xff,

    /* U+0449 "щ" */
    0x92, 0x92, 0x92, 0x92, 0x92, 0x92, 0x92, 0x92,
    0xff, 0x1, 0x1,

    /* U+044A "ъ" */
    0xe0, 0x20, 0x20, 0x20, 0x3e, 0x21, 0x21, 0x21,
    0x3e,

    /* U+044B "ы" */
    0x83, 0x6, 0xf, 0x99, 0xb1, 0x62, 0xcd, 0xf2,

    /* U+044C "ь" */
    0x81, 0x2, 0x4, 0xf, 0xd0, 0x60, 0xc1, 0xfc,

    /* U+044D "э" */
    0x7d, 0x8e, 0x8, 0x13, 0xe0, 0x60, 0xe3, 0x7c,

    /* U+044E "ю" */
    0x8e, 0x91, 0x91, 0x91, 0xf1, 0x91, 0x91, 0x91,
    0x8e,

    /* U+044F "я" */
    0x7f, 0x86, 0xc, 0x1c, 0x2f, 0xd8, 0xa1, 0x82
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 154, .box_w = 1, .box_h = 1, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1, .adv_w = 154, .box_w = 2, .box_h = 12, .ofs_x = 4, .ofs_y = 0},
    {.bitmap_index = 4, .adv_w = 154, .box_w = 4, .box_h = 5, .ofs_x = 2, .ofs_y = 7},
    {.bitmap_index = 7, .adv_w = 154, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 21, .adv_w = 154, .box_w = 7, .box_h = 17, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 36, .adv_w = 154, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 50, .adv_w = 154, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 62, .adv_w = 154, .box_w = 1, .box_h = 5, .ofs_x = 4, .ofs_y = 7},
    {.bitmap_index = 63, .adv_w = 154, .box_w = 5, .box_h = 16, .ofs_x = 3, .ofs_y = -2},
    {.bitmap_index = 73, .adv_w = 154, .box_w = 5, .box_h = 16, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 83, .adv_w = 154, .box_w = 9, .box_h = 8, .ofs_x = 0, .ofs_y = 1},
    {.bitmap_index = 92, .adv_w = 154, .box_w = 8, .box_h = 7, .ofs_x = 1, .ofs_y = 2},
    {.bitmap_index = 99, .adv_w = 154, .box_w = 3, .box_h = 5, .ofs_x = 3, .ofs_y = -3},
    {.bitmap_index = 101, .adv_w = 154, .box_w = 5, .box_h = 1, .ofs_x = 2, .ofs_y = 5},
    {.bitmap_index = 102, .adv_w = 154, .box_w = 3, .box_h = 2, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 103, .adv_w = 154, .box_w = 7, .box_h = 15, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 117, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 128, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 139, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 150, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 161, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 172, .adv_w = 154, .box_w = 6, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 181, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 192, .adv_w = 154, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 204, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 215, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 226, .adv_w = 154, .box_w = 3, .box_h = 9, .ofs_x = 3, .ofs_y = 0},
    {.bitmap_index = 230, .adv_w = 154, .box_w = 4, .box_h = 12, .ofs_x = 2, .ofs_y = -3},
    {.bitmap_index = 236, .adv_w = 154, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 244, .adv_w = 154, .box_w = 7, .box_h = 5, .ofs_x = 1, .ofs_y = 3},
    {.bitmap_index = 249, .adv_w = 154, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 257, .adv_w = 154, .box_w = 6, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 266, .adv_w = 154, .box_w = 8, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 281, .adv_w = 154, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 293, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 304, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 315, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 326, .adv_w = 154, .box_w = 6, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 335, .adv_w = 154, .box_w = 6, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 344, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 355, .adv_w = 154, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 364, .adv_w = 154, .box_w = 6, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 373, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 384, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 395, .adv_w = 154, .box_w = 6, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 404, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 415, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 426, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 437, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 448, .adv_w = 154, .box_w = 7, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 462, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 473, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 484, .adv_w = 154, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 496, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 507, .adv_w = 154, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 519, .adv_w = 154, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 531, .adv_w = 154, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 543, .adv_w = 154, .box_w = 9, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 557, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 568, .adv_w = 154, .box_w = 3, .box_h = 15, .ofs_x = 3, .ofs_y = -2},
    {.bitmap_index = 574, .adv_w = 154, .box_w = 7, .box_h = 15, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 588, .adv_w = 154, .box_w = 3, .box_h = 15, .ofs_x = 3, .ofs_y = -2},
    {.bitmap_index = 594, .adv_w = 154, .box_w = 7, .box_h = 7, .ofs_x = 1, .ofs_y = 5},
    {.bitmap_index = 601, .adv_w = 154, .box_w = 8, .box_h = 1, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 602, .adv_w = 154, .box_w = 3, .box_h = 3, .ofs_x = 3, .ofs_y = 10},
    {.bitmap_index = 604, .adv_w = 154, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 612, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 623, .adv_w = 154, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 631, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 642, .adv_w = 154, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 650, .adv_w = 154, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 662, .adv_w = 154, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 671, .adv_w = 154, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 680, .adv_w = 154, .box_w = 8, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 693, .adv_w = 154, .box_w = 6, .box_h = 16, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 705, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 716, .adv_w = 154, .box_w = 9, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 730, .adv_w = 154, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 739, .adv_w = 154, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 746, .adv_w = 154, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 754, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 765, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 776, .adv_w = 154, .box_w = 7, .box_h = 9, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 784, .adv_w = 154, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 792, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 803, .adv_w = 154, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 811, .adv_w = 154, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 820, .adv_w = 154, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 829, .adv_w = 154, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 838, .adv_w = 154, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 850, .adv_w = 154, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 858, .adv_w = 154, .box_w = 7, .box_h = 15, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 872, .adv_w = 154, .box_w = 1, .box_h = 15, .ofs_x = 4, .ofs_y = -2},
    {.bitmap_index = 874, .adv_w = 154, .box_w = 7, .box_h = 15, .ofs_x = 2, .ofs_y = -2},
    {.bitmap_index = 888, .adv_w = 154, .box_w = 7, .box_h = 3, .ofs_x = 1, .ofs_y = 4},
    {.bitmap_index = 891, .adv_w = 154, .box_w = 5, .box_h = 5, .ofs_x = 2, .ofs_y = 7},
    {.bitmap_index = 895, .adv_w = 154, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 907, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 918, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 929, .adv_w = 154, .box_w = 6, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 938, .adv_w = 154, .box_w = 9, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 954, .adv_w = 154, .box_w = 6, .box_h = 12, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 963, .adv_w = 154, .box_w = 9, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 977, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 988, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 999, .adv_w = 154, .box_w = 7, .box_h = 16, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1013, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1024, .adv_w = 154, .box_w = 8, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1036, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1047, .adv_w = 154, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1056, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1067, .adv_w = 154, .box_w = 6, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1076, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1087, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1098, .adv_w = 154, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1110, .adv_w = 154, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1122, .adv_w = 154, .box_w = 7, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1134, .adv_w = 154, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1146, .adv_w = 154, .box_w = 7, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 1159, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1170, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1181, .adv_w = 154, .box_w = 8, .box_h = 14, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 1195, .adv_w = 154, .box_w = 8, .box_h = 12, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1207, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1218, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1229, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1240, .adv_w = 154, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1252, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1263, .adv_w = 154, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1271, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1282, .adv_w = 154, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1290, .adv_w = 154, .box_w = 6, .box_h = 9, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1297, .adv_w = 154, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 1308, .adv_w = 154, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1316, .adv_w = 154, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1327, .adv_w = 154, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1336, .adv_w = 154, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1343, .adv_w = 154, .box_w = 6, .box_h = 13, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1353, .adv_w = 154, .box_w = 7, .box_h = 9, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1361, .adv_w = 154, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1370, .adv_w = 154, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1378, .adv_w = 154, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1385, .adv_w = 154, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1393, .adv_w = 154, .box_w = 6, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1400, .adv_w = 154, .box_w = 7, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1411, .adv_w = 154, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1419, .adv_w = 154, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1428, .adv_w = 154, .box_w = 8, .box_h = 12, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1440, .adv_w = 154, .box_w = 7, .box_h = 15, .ofs_x = 1, .ofs_y = -3},
    {.bitmap_index = 1454, .adv_w = 154, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1463, .adv_w = 154, .box_w = 7, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 1473, .adv_w = 154, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1481, .adv_w = 154, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1490, .adv_w = 154, .box_w = 8, .box_h = 11, .ofs_x = 1, .ofs_y = -2},
    {.bitmap_index = 1501, .adv_w = 154, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 1510, .adv_w = 154, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1518, .adv_w = 154, .box_w = 7, .box_h = 9, .ofs_x = 2, .ofs_y = 0},
    {.bitmap_index = 1526, .adv_w = 154, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1534, .adv_w = 154, .box_w = 8, .box_h = 9, .ofs_x = 1, .ofs_y = 0},
    {.bitmap_index = 1543, .adv_w = 154, .box_w = 7, .box_h = 9, .ofs_x = 1, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 95, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 176, .range_length = 1, .glyph_id_start = 96,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    },
    {
        .range_start = 1040, .range_length = 64, .glyph_id_start = 97,
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
    .cmap_num = 3,
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
const lv_font_t jetbrains_mono_16 = {
#else
lv_font_t jetbrains_mono_16 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 19,          /*The maximum line height required by the font*/
    .base_line = 3,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
};



#endif /*#if JETBRAINS_MONO_16*/

