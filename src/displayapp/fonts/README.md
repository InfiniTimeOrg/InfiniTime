# Fonts

* [Jetbrains Mono](https://www.jetbrains.com/fr-fr/lp/mono/)
* [Awesome font from LVGL](https://lvgl.io/assets/others/FontAwesome5-Solid+Brands+Regular.woff)
* [Open Sans Light from Google](https://fonts.google.com/specimen/Open+Sans)

## Generate the fonts:

* Open the [LVGL font converter](https://lvgl.io/tools/fontconverter)
* Enter the settings for the font that you wish to convert
* Click on Convert, download the file and place it in `src/DisplayApp/Fonts`

### How to add new symbols:

* Browse [this cheatsheet](https://fontawesome.com/cheatsheet/free/solid) and pick symbols
* For each symbol, add its hex code (0xf641 for the 'Ad' icon, for example) to the *Range* list (Remember to keep this
  readme updated with newest range list)
* Convert this hex value into a UTF-8 code
  using [this site](http://www.ltg.ed.ac.uk/~richard/utf-8.cgi?input=f185&mode=hex)
* Define the new symbols in `src/displayapp/screens/Symbols.h`:

```
static constexpr const char* newSymbol = "\xEF\x86\x85";
```

### Small font

* Name: jetbrains_mono_bold_20
* Size: 20
* Bpp: 1 bit-per-pixel
* Do not enable font compression or horizontal subpixel rendering
* Load the file `JetBrainsMono-Bold.tff` (use the file in this repo to ensure the version matches) and specify the following range: `0x20-0x7e, 0x410-0x44f`
* Add a 2nd font, load the file `FontAwesome5-Solid+Brands+Regular.woff` and specify the following
  range: `0xf293, 0xf294, 0xf244, 0xf240, 0xf242, 0xf243, 0xf241, 0xf54b, 0xf21e, 0xf1e6, 0xf54b, 0xf017, 0xf129, 0xf03a, 0xf185, 0xf560, 0xf001, 0xf3fd, 0xf069, 0xf1fc, 0xf45d, 0xf59f, 0xf5a0, 0xf029, 0xf027, 0xf028, 0xf6a9, 0xf04b, 0xf04c, 0xf048, 0xf051, 0xf095, 0xf3dd, 0xf04d, 0xf2f2, 0xf024, 0xf252, 0xf569, 0xf201, 0xf06e, 0xf015`
* Fix an error in the font conversion.

Replace the following:

    /* U+0030 "0" */
    0x3f, 0x1f, 0xef, 0x3f, 0x87, 0xe1, 0xf8, 0x7f,
    0xdf, 0xf7, 0xe1, 0xf8, 0x7e, 0x1f, 0xcf, 0x7f,
    0x8f, 0xc0,

with

    /* U+0030 "0" */
    0x3f, 0x1f, 0xef, 0x3f, 0x87, 0xe1, 0xf8, 0x7e,
    0xdf, 0xb7, 0xe1, 0xf8, 0x7e, 0x1f, 0xcf, 0x7f,
    0x8f, 0xc0,

(there are two changes: 7f -> 7e and f7 -> b7)

### Medium font

* Name: jetbrains_mono_42
* Size: 42
* Bpp: 1 bit-per-pixel
* Do not enable font compression or horizontal subpixel rendering
* Load the file `JetBrainsMono-Regular.tff` (use the file in this repo to ensure the version matches) and specify the following range: `0x25, 0x30-0x3a`

### Large font

* Name: jetbrains_mono_76
* Size: 76
* Bpp: 1 bit-per-pixel
* Do not enable font compression or horizontal subpixel rendering
* Load the file `JetBrainsMono-Regular.tff` (use the file in this repo to ensure the version matches) and specify the following range: `0x25, 0x2D, 0x2F, 0x30-0x3a`

### Digital watchface font

* Name: jetbrains_mono_extrabold_compressed
* Size: 80
* Bpp: 1 bit-per-pixel
* Do not enable font compression or horizontal subpixel rendering
* Load the file `JetBrainsMono-ExtraBold.tff` (use the file in this repo to ensure the version matches) and specify the following range: `0x30-0x3a`

### PineTimeStyle font

* Name: open_sans_light
* Size: 150
* Bpp: 1 bit-per-pixel
* Do not enable font compression or horizontal subpixel rendering
* Load the file `open_sans_light.tff` (use the file in this repo to ensure the version matches) and specify the following symbols: `0123456789`

### Symbols font (Used in QuickSettings for example)

* Name: lv_font_sys_48
* Size: 48
* Bpp: 1 bit-per-pixel
* Do not enable font compression or horizontal subpixel rendering
* Load the file `icons_sys_48.tff` and specify the following range: `0xe902, 0xe904-0xe907, 0xe90b-0xe90c`

### Navigation font

`navigtion.ttf` is created with the web app [icomoon](https://icomoon.io/app) by importing the svg files from `src/displayapp/icons/navigation/unique` and generating the font. `lv_font_navi_80.json` is a project file for the site, which you can import to add or remove icons.

This font must be generated with the `lv_font_conv` tool, which has additional options not available in the online converter.

`lv_font_conv --font navigation.ttf -r '0xe900-0xe929' --size 80 --format lvgl --bpp 2 -o lv_font_navi_80.c`
