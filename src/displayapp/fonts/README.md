# Fonts

* [Jetbrains Mono](https://www.jetbrains.com/fr-fr/lp/mono/)
* [Awesome font from LVGL](https://lvgl.io/assets/others/FontAwesome5-Solid+Brands+Regular.woff)

## Generate the fonts:

Build and run the docker image with

    docker build -t lv_font_conv .
    docker run -it -v $PWD:/mnt lv_font_conv

this will update the file [`jetbrains_mono_bold_20.c`](./jetbrains_mono_bold_20.c) in the current directory.


> **NOTE**: The fonts can be manually generated without docker with the [LVGL font converter](https://lvgl.io/tools/fontconverter). Use the parameters in the [`Dockerfile`](./Dockerfile) to fill out the form.

### Add new symbols

* Browse the [cheatsheet](https://fontawesome.com/cheatsheet/free/solid) and find your new symbols
* For each symbol, add its hex code (`0xf641` is the 'Ad' icon, for example) to the *range* list in the [`Dockerfile`](./Dockerfile)
* Re-build and run the docker image with the commands above
* Convert the hex value into a UTF-8 code
  using either [this site](http://www.ltg.ed.ac.uk/~richard/utf-8.cgi?input=f185&mode=hex) or python:

    ```py
    print(chr(0xf641).encode("utf-8"))
    ```

* Define the new symbol(s) in [`src/displayapp/screens/Symbols.h`](../screens/Symbols.h):

    ```cpp
    static constexpr const char* newSymbol = "\xEF\x86\x85";
    ```

### Navigation font

To create the `navigtion.ttf` I use the web app [icomoon](https://icomoon.io/app)
this app can import the svg files from the folder `src/displayapp/icons/navigation/unique` and creat a ttf file the
project for the site is *lv_font_navi_80.json* you can import it to add or remove icons

You can also use the online LVGL tool to create the .c

ttf file : navigation.ttf name : lv_font_navi_80 size : 80px Bpp : 2 bit-per-pixel range : 0xe900-0xe929

```
$lv_font_conv --font navigation.ttf -r '0xe900-0xe929' --size 80 --format lvgl --bpp 2 --no-prefilter -o lv_font_navi_80.c
```
