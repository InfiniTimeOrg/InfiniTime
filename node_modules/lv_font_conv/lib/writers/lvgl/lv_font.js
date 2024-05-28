'use strict';


const path = require('path');

const Font = require('../../font/font');
const Head = require('./lv_table_head');
const Cmap = require('./lv_table_cmap');
const Glyf = require('./lv_table_glyf');
const Kern = require('./lv_table_kern');
const AppError = require('../../app_error');


class LvFont extends Font {
  constructor(fontData, options) {
    super(fontData, options);

    const ext = path.extname(options.output);
    this.font_name = path.basename(options.output, ext);

    if (options.bpp === 3 & options.no_compress) {
      throw new AppError('LittlevGL supports "--bpp 3" with compression only');
    }
  }

  init_tables() {
    this.head = new Head(this);
    this.glyf = new Glyf(this);
    this.cmap = new Cmap(this);
    this.kern = new Kern(this);
  }

  large_format_guard() {
    let guard_required = false;
    let glyphs_bin_size = 0;

    this.glyf.lv_data.forEach(d => {
      glyphs_bin_size += d.bin.length;

      if (d.glyph.bbox.width > 255 ||
          d.glyph.bbox.height > 255 ||
          Math.abs(d.glyph.bbox.x) > 127 ||
          Math.abs(d.glyph.bbox.y) > 127 ||
          Math.round(d.glyph.advanceWidth * 16) > 4096) {
        guard_required = true;
      }
    });

    if (glyphs_bin_size > 1024 * 1024) guard_required = true;

    if (!guard_required) return '';

    return `
#if (LV_FONT_FMT_TXT_LARGE == 0)
#  error "Too large font or glyphs in ${this.font_name.toUpperCase()}. Enable LV_FONT_FMT_TXT_LARGE in lv_conf.h")
#endif
`.trimLeft();
  }

  toLVGL() {
    let guard_name =  this.font_name.toUpperCase();

    return `/*******************************************************************************
 * Size: ${this.src.size} px
 * Bpp: ${this.opts.bpp}
 * Opts: ${process.argv.slice(2).join(' ')}
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "${this.opts.lv_include || 'lvgl/lvgl.h'}"
#endif

#ifndef ${guard_name}
#define ${guard_name} 1
#endif

#if ${guard_name}

${this.glyf.toLVGL()}

${this.cmap.toLVGL()}

${this.kern.toLVGL()}

${this.head.toLVGL()}

${this.large_format_guard()}

#endif /*#if ${guard_name}*/

`;
  }
}


module.exports = LvFont;
