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
