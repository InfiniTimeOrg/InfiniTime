#Fonts
* [Jetbrains Mono](https://www.jetbrains.com/fr-fr/lp/mono/)
* [Awesome font from LVGL](https://lvgl.io/assets/others/FontAwesome5-Solid+Brands+Regular.woff)

## Generate the fonts:

 * Open the [LVGL font converter](https://lvgl.io/tools/fontconverter)
 * Name : jetbrains_mono_bold_20
 * Size : 20
 * Bpp : 1 bit-per-pixel
 * Do not enable font compression and horizontal subpixel hinting
 * Load the file `JetBrainsMono-Bold.tff` and specify the following range : `0x20-0x7f, 0x410-0x44f`
 * Add a 2nd font, load the file `FontAwesome5-Solid+Brands+Regular.woff` and specify the following range : `0xf293, 0xf294, 0xf244, 0xf240, 0xf242, 0xf243, 0xf241, 0xf54b, 0xf21e, 0xf1e6, 0xf54b, 0xf017, 0xf129, 0xf03a, 0xf185, 0xf560, 0xf001, 0xf3fd, 0xf069, 0xf1fc, 0xf45d, 0xf59f, 0xf5a0, 0xf029, 0xf027, 0xf028, 0xf6a9, 0xf04b, 0xf04c, 0xf048, 0xf051, 0xf095, 0xf3dd, 0xf04d, 0xf2f2, 0xf024`
 * Click on Convert, and download the file `jetbrains_mono_bold_20.c` and copy it in `src/DisplayApp/Fonts`
  
Add new symbols:
 * Browse the [cheatsheet](https://fontawesome.com/cheatsheet/free/solid) and find your new symbols
 * For each symbol, add its hex code (0xf641 for the 'Ad' icon, for example) to the *Range* list (Remember to keep this readme updated with newest range list)
 * Convert this hex value into a UTF-8 code using [this site](http://www.ltg.ed.ac.uk/~richard/utf-8.cgi?input=f185&mode=hex)
 * Define the new symbols in `src/DisplayApp/Screens/Symbols.h`: 
```
static constex char* newSymbol = "\xEF\x86\x85";
```

#### Navigation font

navigation.ttf
0xe900-0xe929

0xe900,0xe901,0xe902,0xe903,0xe904,0xe905,0xe906,0xe907,0xe908,0xe909,0xe90a,0xe90b,0xe90c,0xe90d,0xe90e,0xe90f,0xe910,0xe911,0xe912,0xe913,0xe914,0xe915,0xe916,0xe917,0xe918,0xe919,0xe91a,0xe91b,0xe91c,0xe91d,0xe91e,0xe91f,0xe920,0xe921,0xe922,0xe923,0xe924,0xe925,0xe926,0xe927,0xe928,0xe929

lv_font_conv --font navigation.ttf -r '0xe900-0xe929' --size 70 --format lvgl --bpp 2 --no-prefilter -o lv_font_navi_70.c 