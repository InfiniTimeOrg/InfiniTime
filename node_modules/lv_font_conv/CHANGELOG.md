1.5.2 / 2021-07-18
------------------

- Fixed lvgl version check for v8+, #64.


1.5.1 / 2021-04-06
------------------

- Fixed fail of CMAP generation for edge cases, #62.
- Dev deps bump.


1.5.0 / 2021-03-08
------------------

- More `const` in generated font (for v8+), #59.


1.4.1 / 2021-01-26
------------------

- Fix charcodes padding in comments, #54.


1.4.0 / 2021-01-03
------------------

- Added OTF fonts support.
- Added `--use-color-info` for limited multi-tone glyphs support.


1.3.1 / 2020-12-28
------------------

- Unify `lvgl.h` include.
- Updated repo refs (littlevgl => lvgl).
- Deps bump.
- Moved CI to github actions.


1.3.0 / 2020-10-25
------------------

- Drop `lodash` use.
- Deps bump.


1.2.1 / 2020-10-24
------------------

- Reduced npm package size (drop unneeded files before publish).


1.2.0 / 2020-10-24
------------------

- Bump FreeType to 2.10.4.
- Bundle dependencies to npm package.


1.1.3 / 2020-09-22
------------------

- lvgl: added `LV_FONT_FMT_TXT_LARGE` check or very large fonts.


1.1.2 / 2020-08-23
------------------

- Fix: skip `glyph.advanceWidth` for monospace fonts, #43.
- Spec fix: version size should be 4 bytes, #44.
- Spec fix: bbox x/y bits => unsigned, #45.
- Bump argparse.
- Cleanup help formatter.


1.1.1 / 2020-08-01
------------------

- `--version` should show number from `package.json`.


1.1.0 / 2020-07-27
------------------

- Added `post.underlinePosition` & `post.underlineThickness` info to font header.


1.0.0 / 2020-06-26
------------------

- Maintenance release.
- Set package version 1.x, to label package as stable.
- Deps bump.


0.4.3 / 2020-03-05
------------------

- Enabled `--bpp 8` mode.


0.4.2 / 2020-01-05
------------------

- Added `--lv_include` option to set alternate `lvgl.h` path.
- Added guards to hide `.subpx` property for lvgl 6.0 (supported from 6.1 only), #32.
- Dev deps bump


0.4.1 / 2019-12-09
------------------

- Allow memory growth for FreeType build, #29.
- Dev deps bump.
- Web build update.


0.4.0 / 2019-11-29
------------------

- Note, this release is for lvgl 6.1 and has potentially breaking changes
  (see below). If you have compatibility issues with lvgl 6.0 - use previous
  versions or update your code.
- Spec change: added subpixels info field to font header (header size increased).
- Updated `bin` & `lvgl` writers to match new spec.
- lvgl: fixed data type for kerning values (needs appropriate update
  in LittlevGL 6.1+).
- Fix errors display (disable emscripten error catcher).


0.3.1 / 2019-10-24
------------------

- Fixed "out of range" error for big `--size`.


0.3.0 / 2019-10-12
------------------

- Added beta options `--lcd` & `--lcd-v` for subpixel rendering (still need
  header info update).
- Added FreeType data properties to dump info.
- Fixed glyph width (missed fractional part after switch to FreeType).
- Fixed missed sigh for negative X/Y bitmap offsets.
- Deps bump.


0.2.0 / 2019-09-26
------------------

- Use FreeType renderer. Should solve all regressions, reported in 0.1.0.
- Enforced light autohinting (horizontal lines only).
- Use special hinter for monochrome output (improve quality).
- API changed to async.
- Fix: added missed `.bitmap_format` field to lvgl writer.
- Fix: changed struct fields init order to match declaration, #25.


0.1.0 / 2019-09-03
------------------

- First release.
