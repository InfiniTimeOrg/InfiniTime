'use strict';


const Head = require('../../font/table_head');


class LvHead extends Head {
  constructor(font) {
    super(font);
  }

  kern_ref() {
    const f = this.font;

    if (!f.hasKerning()) {
      return {
        scale:   '0',
        dsc:     'NULL',
        classes: '0'
      };
    }

    if (!f.kern.should_use_format3()) {
      return {
        scale: `${Math.round(f.kerningScale * 16)}`,
        dsc: '&kern_pairs',
        classes: '0'
      };
    }

    return {
      scale: `${Math.round(f.kerningScale * 16)}`,
      dsc: '&kern_classes',
      classes: '1'
    };
  }

  toLVGL() {
    const f = this.font;
    const kern = this.kern_ref();
    const subpixels = (f.subpixels_mode === 0) ? 'LV_FONT_SUBPX_NONE' :
      (f.subpixels_mode === 1) ? 'LV_FONT_SUBPX_HOR' : 'LV_FONT_SUBPX_VER';

    return `
/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LV_VERSION_CHECK(8, 0, 0)
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = ${kern.dsc},
    .kern_scale = ${kern.scale},
    .cmap_num = ${f.cmap.toBin().readUInt32LE(8)},
    .bpp = ${f.opts.bpp},
    .kern_classes = ${kern.classes},
    .bitmap_format = ${f.glyf.getCompressionCode()},
#if LV_VERSION_CHECK(8, 0, 0)
    .cache = &cache
#endif
};


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LV_VERSION_CHECK(8, 0, 0)
const lv_font_t ${f.font_name} = {
#else
lv_font_t ${f.font_name} = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = ${f.src.ascent - f.src.descent},          /*The maximum line height required by the font*/
    .base_line = ${-f.src.descent},             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = ${subpixels},
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = ${f.src.underlinePosition},
    .underline_thickness = ${f.src.underlineThickness},
#endif
    .dsc = &font_dsc           /*The custom font data. Will be accessed by \`get_glyph_bitmap/dsc\` */
};
`.trim();
  }
}


module.exports = LvHead;
