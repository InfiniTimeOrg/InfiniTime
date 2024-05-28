lv_font_conv - font convertor to compact bitmap format
======================================================

[![CI](https://github.com/lvgl/lv_font_conv/workflows/CI/badge.svg?branch=master)](https://github.com/lvgl/lv_font_conv/actions)
[![NPM version](https://img.shields.io/npm/v/lv_font_conv.svg?style=flat)](https://www.npmjs.org/package/lv_font_conv)

Converts TTF/WOFF/OTF fonts to __[compact format](https://github.com/lvgl/lv_font_conv/blob/master/doc/font_spec.md)__, suitable for small embedded systems. Main features are:

- Allows bitonal and anti-aliased glyphs (1-4 bits per pixel).
- Preserves kerning info.
- Compression.
- Users can select required glyphs only (subsetting).
- Multiple font sources can be merged.
- Simple CLI interface, easy to integrate into external build systems.


## Install the script

[node.js](https://nodejs.org/en/download/) v10+ required.

Global install of the last version, execute as "lv_font_conv"

```sh
# install release from npm registry
npm i lv_font_conv -g
# install from github's repo, master branch
npm i lvgl/lv_font_conv -g
```

**run via [npx](https://www.npmjs.com/package/npx) without install**

```sh
# run from npm registry
npx lv_font_conv -h
# run from github master
npx github:lvgl/lv_font_conv -h
```

Note, runing via `npx` may take some time until modules installed, be patient.


## CLI params

Common:

- `--bpp` - bits per pixel (antialiasing).
- `--size` - output font size (pixels).
- `-o`, `--output` - output path (file or directory, depends on format).
- `--format` - output format.
  - `--format dump` - dump glyph images and font info, useful for debug.
  - `--format bin` - dump font in binary form (as described in [spec](https://github.com/lvgl/lv_font_conv/blob/master/doc/font_spec.md)).
  - `--format lvgl` - dump font in [LittlevGL](https://github.com/lvgl/lvgl) format.
- `--force-fast-kern-format` - always use more fast kering storage format,
  at cost of some size. If size difference appears, it will be displayed.
- `--lcd` - generate bitmaps with 3x horizontal resolution, for subpixel
  smoothing.
- `--lcd-v` - generate bitmaps with 3x vertical resolution, for subpixel
  smoothing.
- `--use-color-info` - try to use glyph color info from font to create
  grayscale icons. Since gray tones are emulated via transparency, result
  will be good on contrast background only.
- `--lv-include` - only with `--format lvgl`, set alternate path for `lvgl.h`.

Per font:

- `--font` - path to font file (ttf/woff/woff2/otf). May be used multiple time for
  merge.
- `-r`, `--range` - single glyph or range + optional mapping, belongs to
  previously declared `--font`. Can be used multiple times. Examples:
  - `-r 0x1F450` - single value, dec or hex format.
  - `-r 0x1F450-0x1F470` - range.
  - `-r '0x1F450=>0xF005'` - single glyph with mapping.
  - `-r '0x1F450-0x1F470=>0xF005'` - range with mapping.
  - `-r 0x1F450 -r 0x1F451-0x1F470` - 2 ranges.
  - `-r 0x1F450,0x1F451-0x1F470` - the same as above, but defined with single `-r`.
- `--symbols` - list of characters to copy (instead of numeric format in `-r`).
  - `--symbols 0123456789.,` - extract chars to display numbers.
- `--autohint-off` - do not force autohinting ("light" is on by default).
- `--autohint-strong` - use more strong autohinting (will break kerning).

Additional debug options:

- `--no-compress` - disable built-in RLE compression.
- `--no-prefilter` - disable bitmap lines filter (XOR), used to improve
  compression ratio.
- `--no-kerning` - drop kerning info to reduce size (not recommended).
- `--full-info` - don't shorten 'font_info.json' (include pixels data).


## Examples

Merge english from Roboto Regular and icons from Font Awesome, and show debug
info:

`env DEBUG=* lv_font_conv --font Roboto-Regular.ttf -r 0x20-0x7F --font FontAwesome.ttf -r 0xFE00=>0x81 --size 16 --format bin --bpp 3 --no-compress -o output.font`

Merge english & russian from Roboto Regular, and show debug info:

`env DEBUG=* lv_font_conv --font Roboto-Regular.ttf -r 0x20-0x7F -r 0x401,0x410-0x44F,0x451 --size 16 --format bin --bpp 3 --no-compress -o output.font`

Dump all Roboto glyphs to inspect icons and font details:

`lv_font_conv --font Roboto-Regular.ttf -r 0x20-0x7F --size 16 --format dump --bpp 3 -o ./dump`

**Note**. Option `--no-compress` exists temporary, to avoid confusion until LVGL
adds compression support.


## Technical notes

### Supported output formats

1. **bin** - universal binary format, as described in https://github.com/lvgl/lv_font_conv/tree/master/doc.
2. **lvgl** - format for LittlevGL, C file. Has minor limitations and a bit
   bigger size, because C does not allow to effectively define relative offsets
   in data blocks.
3. **dump** - create folder with each glyph in separate image, and other font
   data as `json`. Useful for debug.

### Merged font metrics

When multiple fonts merged into one, sources can have different metrics. Result
will follow principles below:

1. No scaling. Glyphs will have exactly the same size, as intended by font authors.
2. The same baseline.
3. `OS/2` metrics (`sTypoAscender`, `sTypoDescender`, `sTypoLineGap`) will be
   used from the first font in list.
4. `hhea`  metrics (`ascender`, `descender`), defined as max/min point of all
   font glyphs, are recalculated, according to new glyphs set.


## Development

Current package includes WebAssembly build of FreeType with some helper
functions. Everything is wrapped into Docker and requires zero knowledge about
additional tools install. See `package.json` for additional commands. You may
need those if decide to upgrade FreeType or update helpers.

This builds image with emscripten & freetype, usually should be done only once:

```
npm run build:dockerimage
```

This compiles helpers and creates WebAssembly files:

```
npm run build:freetype
```
