# Fonts

- [Jetbrains Mono](https://www.jetbrains.com/lp/mono/)
- [Font Awesome](https://fontawesome.com/v5/cheatsheet/free/solid)
- [Open Sans Light](https://fonts.google.com/specimen/Open+Sans)
- [Material Symbols](https://fonts.google.com/icons)

### How to add new symbols:

- Browse the cheat sheets and pick symbols
  - [Font Awesome](https://fontawesome.com/v5/cheatsheet/free/solid)
  - [Material Symbols](https://fonts.google.com/icons)
- For each symbol, add its hex code (0xf641 for the 'Ad' icon, for example) to the *Range* list in the `fonts.json` file
- Convert this hex value into a UTF-8 code
  using [this site](http://www.ltg.ed.ac.uk/~richard/utf-8.cgi?input=f185&mode=hex)
- Define the new symbols in `src/displayapp/screens/Symbols.h`:

```
static constexpr const char* newSymbol = "\xEF\x86\x85";
```

### the config file format:

inside `fonts`, there is a dictionary of fonts,
and for each font there is:

- sources - list of file,range(,symbols) wanted (as a dictionary of those)
- bpp - bits per pixel.
- size - size.
- patches - list of extra "patches" to run: a path to a .patch file. (may be relative)
- compress - optional. default disabled. add `"compress": true` to enable

### Navigation font

`navigtion.ttf` is created with the web app [icomoon](https://icomoon.io/app) by importing the svg files from `src/displayapp/icons/navigation/unique` and generating the font. `lv_font_navi_80.json` is a project file for the site, which you can import to add or remove icons.

To save space in the internal flash memory, the navigation icons are now moved into the external flash memory. To do this, the TTF font is converted into pictures (1 for each symbol). Those pictures are then concatenated into 2 big pictures (we need two files since LVGL supports maximum 2048px width/height). At runtime, a map is used to locate the desired icon in the corresponding file at a specific offset. 

Here is the command to convert the TTF font in PNG picture:

```shell
convert -background none -fill white -font navigation.ttf -pointsize 80 -gravity center label:"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"  navigation0.png

convert -background none -fill white -font navigation.ttf -pointsize 80 -gravity center label:"\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"  navigation1.png
```

*Please note that the characters after `label:` are UTF-8 characters and might not be displayed correctly in this document.*

The characters in the TTF font range from `0xEEA480` to `0xEEA4A9`. Characters from `0xEEA480` to `0xEEA498` are stored in `navigation0.png` and the others in `navigation1.png`. Each character is 80px height so displaying a specific character consists in multiplying its index in the file by -80 and use this value as the offset when calling `lv_img_set_offset_y()`.
