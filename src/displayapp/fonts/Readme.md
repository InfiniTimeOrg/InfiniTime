#Fonts
* [Jetbrains Mono](https://www.jetbrains.com/fr-fr/lp/mono/)
* [Awesome font from LVGL](https://lvgl.io/assets/others/FontAwesome5-Solid+Brands+Regular.woff)

## Generate the fonts:

 * Open the [LVGL font converter](https://lvgl.io/tools/fontconverter)
 * Name : jetbrains_mono_bold_20
 * Size : 20
 * Bpp : 1 bit-per-pixel
 * Do not enable font compression and horizontal subpixel hinting
 * Load the file `JetBrainsMono-Bold.woff` and specify the following range : `0x20-0x7f, 0x410-0x44f`
 * Add a 2nd font, load the file `FontAwesome5-Solid+Brands+Regular.woff` and specify the following range : `0xf293, 0xf294, 0xf244, 0xf240, 0xf242, 0xf243, 0xf241, 0xf54b, 0xf21e, 0xf1e6, 0xf54b, 0xf017, 0xf129, 0xf03a, 0xf185, 0xf560, 0xf001, 0xf3fd, 0xf069, 0xf1fc`
 * Click on Convert, and download the file `jetbrains_mono_bold_20.c` and copy it in `src/DisplayApp/Fonts`
  
Add new symbols:
 * Browse the [cheatsheet](https://fontawesome.com/cheatsheet/free/solid) and find your new symbols
 * For each symbol, add its hex code (0xf641 for the 'Ad' icon, for example) to the *Range* list
 * Convert this hex value into a UTF-8 code using [this site](http://www.ltg.ed.ac.uk/~richard/utf-8.cgi?input=f185&mode=hex)
 * Define the new symbols in `src/DisplayApp/Screens/Symbols.h`: 
```
static constex char* newSymbol = "\xEF\x86\x85";
```
