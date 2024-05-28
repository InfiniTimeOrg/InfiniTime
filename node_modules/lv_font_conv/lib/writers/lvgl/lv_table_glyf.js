'use strict';


const { BitStream } = require('bit-buffer');
const u = require('../../utils');
const Glyf = require('../../font/table_glyf');


class LvGlyf extends Glyf {
  constructor(font) {
    super(font);

    this.lv_data = [];
    this.lv_compiled = false;
  }

  lv_bitmap(glyph) {
    const buf = Buffer.alloc(100 + glyph.bbox.width * glyph.bbox.height * 4);
    const bs = new BitStream(buf);
    bs.bigEndian = true;

    const pixels = this.font.glyf.pixelsToBpp(glyph.pixels);

    this.font.glyf.storePixels(bs, pixels);

    const glyph_bitmap = Buffer.alloc(bs.byteIndex);
    buf.copy(glyph_bitmap, 0, 0, bs.byteIndex);

    return glyph_bitmap;
  }

  lv_compile() {
    if (this.lv_compiled) return;

    this.lv_compiled = true;

    const f = this.font;
    this.lv_data = [];
    let offset = 0;

    f.src.glyphs.forEach(g => {
      const id = f.glyph_id[g.code];
      const bin = this.lv_bitmap(g);
      this.lv_data[id] = {
        bin,
        offset,
        glyph: g
      };
      offset += bin.length;
    });
  }

  to_lv_bitmaps() {
    this.lv_compile();

    let result = [];
    this.lv_data.forEach((d, idx) => {
      if (idx === 0) return;
      const code_hex = d.glyph.code.toString(16).toUpperCase();
      const code_str = JSON.stringify(String.fromCodePoint(d.glyph.code));

      let txt = `    /* U+${code_hex.padStart(4, '0')} ${code_str} */
${u.long_dump(d.bin, { hex: true })}`;

      if (idx < this.lv_data.length - 1) {
        // skip comma for zero data
        txt += d.bin.length ? ',\n\n' : '\n';
      }

      result.push(txt);
    });

    return result.join('');
  }

  to_lv_glyph_dsc() {
    this.lv_compile();

    /* eslint-disable max-len */

    let result = [ '    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */' ];

    this.lv_data.forEach(d => {
      const idx = d.offset,
            adv_w = Math.round(d.glyph.advanceWidth * 16),
            h = d.glyph.bbox.height,
            w = d.glyph.bbox.width,
            x = d.glyph.bbox.x,
            y = d.glyph.bbox.y;
      result.push(`    {.bitmap_index = ${idx}, .adv_w = ${adv_w}, .box_w = ${w}, .box_h = ${h}, .ofs_x = ${x}, .ofs_y = ${y}}`);
    });

    return result.join(',\n');
  }


  toLVGL() {
    return `
/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
${this.to_lv_bitmaps()}
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
${this.to_lv_glyph_dsc()}
};
`.trim();
  }
}


module.exports = LvGlyf;
