#include <emscripten.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H

static void set_js_variable(char* name, int value) {
  char buffer[strlen(name) + 32];
  sprintf(buffer, "Module.%s = %d;", name, value);
  emscripten_run_script(buffer);
}

// Expose constants, used in calls from js
void init_constants()
{
  set_js_variable("FT_LOAD_DEFAULT",               FT_LOAD_DEFAULT);
  set_js_variable("FT_LOAD_NO_HINTING",            FT_LOAD_NO_HINTING);
  set_js_variable("FT_LOAD_RENDER",                FT_LOAD_RENDER);
  set_js_variable("FT_LOAD_FORCE_AUTOHINT",        FT_LOAD_FORCE_AUTOHINT);
  set_js_variable("FT_LOAD_PEDANTIC",              FT_LOAD_PEDANTIC);
  set_js_variable("FT_LOAD_MONOCHROME",            FT_LOAD_MONOCHROME);
  set_js_variable("FT_LOAD_NO_AUTOHINT",           FT_LOAD_NO_AUTOHINT);
  set_js_variable("FT_LOAD_COLOR",                 FT_LOAD_COLOR);

  set_js_variable("FT_LOAD_TARGET_NORMAL",         FT_LOAD_TARGET_NORMAL);
  set_js_variable("FT_LOAD_TARGET_LIGHT",          FT_LOAD_TARGET_LIGHT);
  set_js_variable("FT_LOAD_TARGET_MONO",           FT_LOAD_TARGET_MONO);
  set_js_variable("FT_LOAD_TARGET_LCD",            FT_LOAD_TARGET_LCD);
  set_js_variable("FT_LOAD_TARGET_LCD_V",          FT_LOAD_TARGET_LCD_V);

  set_js_variable("FT_RENDER_MODE_NORMAL",         FT_RENDER_MODE_NORMAL);
  set_js_variable("FT_RENDER_MODE_MONO",           FT_RENDER_MODE_MONO);
  set_js_variable("FT_RENDER_MODE_LCD",            FT_RENDER_MODE_LCD);
  set_js_variable("FT_RENDER_MODE_LCD_V",          FT_RENDER_MODE_LCD_V);

  set_js_variable("FT_KERNING_DEFAULT",            FT_KERNING_DEFAULT);
  set_js_variable("FT_KERNING_UNFITTED",           FT_KERNING_UNFITTED);
  set_js_variable("FT_KERNING_UNSCALED",           FT_KERNING_UNSCALED);

  set_js_variable("FT_SFNT_OS2",                   FT_SFNT_OS2);

  set_js_variable("FT_FACE_FLAG_COLOR",            FT_FACE_FLAG_COLOR);

  set_js_variable("FT_PIXEL_MODE_MONO",            FT_PIXEL_MODE_MONO);
  set_js_variable("FT_PIXEL_MODE_BGRA",            FT_PIXEL_MODE_BGRA);

  set_js_variable("OFFSET_FACE_GLYPH",             offsetof(FT_FaceRec, glyph));
  set_js_variable("OFFSET_FACE_UNITS_PER_EM",      offsetof(FT_FaceRec, units_per_EM));
  set_js_variable("OFFSET_FACE_ASCENDER",          offsetof(FT_FaceRec, ascender));
  set_js_variable("OFFSET_FACE_DESCENDER",         offsetof(FT_FaceRec, descender));
  set_js_variable("OFFSET_FACE_HEIGHT",            offsetof(FT_FaceRec, height));
  set_js_variable("OFFSET_FACE_FACE_FLAGS",        offsetof(FT_FaceRec, face_flags));

  set_js_variable("OFFSET_GLYPH_BITMAP_WIDTH",        offsetof(FT_GlyphSlotRec, bitmap.width));
  set_js_variable("OFFSET_GLYPH_BITMAP_ROWS",         offsetof(FT_GlyphSlotRec, bitmap.rows));
  set_js_variable("OFFSET_GLYPH_BITMAP_PITCH",        offsetof(FT_GlyphSlotRec, bitmap.pitch));
  set_js_variable("OFFSET_GLYPH_BITMAP_BUFFER",       offsetof(FT_GlyphSlotRec, bitmap.buffer));
  set_js_variable("OFFSET_GLYPH_BITMAP_NUM_GRAYS",    offsetof(FT_GlyphSlotRec, bitmap.num_grays));
  set_js_variable("OFFSET_GLYPH_BITMAP_PIXEL_MODE",   offsetof(FT_GlyphSlotRec, bitmap.pixel_mode));
  set_js_variable("OFFSET_GLYPH_BITMAP_PALETTE_MODE", offsetof(FT_GlyphSlotRec, bitmap.palette_mode));

  set_js_variable("OFFSET_GLYPH_METRICS_WIDTH",          offsetof(FT_GlyphSlotRec, metrics.width));
  set_js_variable("OFFSET_GLYPH_METRICS_HEIGHT",         offsetof(FT_GlyphSlotRec, metrics.height));
  set_js_variable("OFFSET_GLYPH_METRICS_HORI_BEARING_X", offsetof(FT_GlyphSlotRec, metrics.horiBearingX));
  set_js_variable("OFFSET_GLYPH_METRICS_HORI_BEARING_Y", offsetof(FT_GlyphSlotRec, metrics.horiBearingY));
  set_js_variable("OFFSET_GLYPH_METRICS_HORI_ADVANCE",   offsetof(FT_GlyphSlotRec, metrics.horiAdvance));
  set_js_variable("OFFSET_GLYPH_METRICS_VERT_BEARING_X", offsetof(FT_GlyphSlotRec, metrics.vertBearingX));
  set_js_variable("OFFSET_GLYPH_METRICS_VERT_BEARING_Y", offsetof(FT_GlyphSlotRec, metrics.vertBearingY));
  set_js_variable("OFFSET_GLYPH_METRICS_VERT_ADVANCE",   offsetof(FT_GlyphSlotRec, metrics.vertAdvance));

  set_js_variable("OFFSET_GLYPH_BITMAP_LEFT",            offsetof(FT_GlyphSlotRec, bitmap_left));
  set_js_variable("OFFSET_GLYPH_BITMAP_TOP",             offsetof(FT_GlyphSlotRec, bitmap_top));
  set_js_variable("OFFSET_GLYPH_INDEX",                  offsetof(FT_GlyphSlotRec, glyph_index));
  set_js_variable("OFFSET_GLYPH_LINEAR_HORI_ADVANCE",    offsetof(FT_GlyphSlotRec, linearHoriAdvance));
  set_js_variable("OFFSET_GLYPH_LINEAR_VERT_ADVANCE",    offsetof(FT_GlyphSlotRec, linearVertAdvance));
  set_js_variable("OFFSET_GLYPH_ADVANCE_X",              offsetof(FT_GlyphSlotRec, advance.x));
  set_js_variable("OFFSET_GLYPH_ADVANCE_Y",              offsetof(FT_GlyphSlotRec, advance.y));
  set_js_variable("OFFSET_GLYPH_LSB_DELTA",              offsetof(FT_GlyphSlotRec, lsb_delta));
  set_js_variable("OFFSET_GLYPH_RSB_DELTA",              offsetof(FT_GlyphSlotRec, rsb_delta));

  set_js_variable("OFFSET_TT_OS2_ASCENDER",        offsetof(TT_OS2, sTypoAscender));
  set_js_variable("OFFSET_TT_OS2_DESCENDER",       offsetof(TT_OS2, sTypoDescender));
  set_js_variable("OFFSET_TT_OS2_LINEGAP",         offsetof(TT_OS2, sTypoLineGap));
}
