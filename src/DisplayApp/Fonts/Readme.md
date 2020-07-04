#Fonts
* [Jetbrains Mono](https://www.jetbrains.com/fr-fr/lp/mono/)
* [Awesome font from LVGL](https://lvgl.io/assets/others/FontAwesome5-Solid+Brands+Regular.woff)

## Generate the fonts:

 * Open the [LVGL font converter](https://lvgl.io/tools/fontconverter)
 * Name : jetbrains_mono_bold_20
 * Size : 20
 * Bpp : 1 bit-per-pixel
 * Do not enable font compression and horizontal subpixel hinting
 * Load the file `JetBrainsMono-Bold.woff` and specify the following range : `0x20-0x7f`
 * Add a 2nd font, load the file `FontAwesome5-Solid+Brands+Regular.woff` and specify the following range : `0xf293, 0xf294, 0xf244, 0xf240, 0xf242, 0xf243, 0xf241, 0xf54b, 0xf21e, 0xf1e6, 0xf54b, 0xf017, 0xf129, 0xf03a, 0xf185`
 * Click on Convert, and download the file `jetbrains_mono_bold_20.c` and copy it in `src/DisplayApp/Fonts`
  