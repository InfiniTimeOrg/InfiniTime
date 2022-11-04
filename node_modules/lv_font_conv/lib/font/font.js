// Font class to generate tables
'use strict';

const u = require('../utils');
const debug = require('debug')('font');
const Head = require('./table_head');
const Cmap = require('./table_cmap');
const Glyf = require('./table_glyf');
const Loca = require('./table_loca');
const Kern = require('./table_kern');

class Font {
  constructor(fontData, options) {
    this.src = fontData;

    this.opts = options;

    // Map chars to IDs (zero is reserved)
    this.glyph_id = { 0: 0 };

    this.last_id = 1;
    this.createIDs();
    debug(`last_id: ${this.last_id}`);

    this.init_tables();

    this.minY = Math.min(...this.src.glyphs.map(g => g.bbox.y));
    debug(`minY: ${this.minY}`);
    this.maxY = Math.max(...this.src.glyphs.map(g => g.bbox.y + g.bbox.height));
    debug(`maxY: ${this.maxY}`);

    // 0 => 1 byte, 1 => 2 bytes
    this.glyphIdFormat = Math.max(...Object.values(this.glyph_id)) > 255 ? 1 : 0;
    debug(`glyphIdFormat: ${this.glyphIdFormat}`);

    // 1.0 by default, will be stored in font as FP12.4
    this.kerningScale = 1.0;
    let kerningMax = Math.max(...this.src.glyphs.map(g => Object.values(g.kerning).map(Math.abs)).flat());
    if (kerningMax >= 7.5) this.kerningScale = Math.ceil(kerningMax / 7.5 * 16) / 16;
    debug(`kerningScale: ${this.kerningScale}`);

    // 0 => int, 1 => FP4
    this.advanceWidthFormat = this.hasKerning() ? 1 : 0;
    debug(`advanceWidthFormat: ${this.advanceWidthFormat}`);

    this.xy_bits = Math.max(...this.src.glyphs.map(g => Math.max(
      u.signed_bits(g.bbox.x), u.signed_bits(g.bbox.y)
    )));
    debug(`xy_bits: ${this.xy_bits}`);

    this.wh_bits = Math.max(...this.src.glyphs.map(g => Math.max(
      u.unsigned_bits(g.bbox.width), u.unsigned_bits(g.bbox.height)
    )));
    debug(`wh_bits: ${this.wh_bits}`);

    this.advanceWidthBits = Math.max(...this.src.glyphs.map(
      g => u.signed_bits(this.widthToInt(g.advanceWidth))
    ));
    debug(`advanceWidthBits: ${this.advanceWidthBits}`);

    let glyphs = this.src.glyphs;

    this.monospaced = glyphs.every((v, i, arr) => v.advanceWidth === arr[0].advanceWidth);
    debug(`monospaced: ${this.monospaced}`);

    // This should stay in the end, because depends on previous variables
    // 0 => 2 bytes, 1 => 4 bytes
    this.indexToLocFormat = this.glyf.getSize() > 65535 ? 1 : 0;
    debug(`indexToLocFormat: ${this.indexToLocFormat}`);

    this.subpixels_mode = options.lcd ? 1 : (options.lcd_v ? 2 : 0);
    debug(`subpixels_mode: ${this.subpixels_mode}`);
  }

  init_tables() {
    this.head = new Head(this);
    this.glyf = new Glyf(this);
    this.cmap = new Cmap(this);
    this.loca = new Loca(this);
    this.kern = new Kern(this);
  }

  createIDs() {
    // Simplified, don't check dupes
    this.last_id = 1;

    for (let i = 0; i < this.src.glyphs.length; i++) {
      // reserve zero for special cases
      this.glyph_id[this.src.glyphs[i].code] = this.last_id;
      this.last_id++;
    }
  }

  hasKerning() {
    if (this.opts.no_kerning) return false;

    for (let glyph of this.src.glyphs) {
      if (glyph.kerning && Object.keys(glyph.kerning).length) return true;
    }
    return false;
  }

  // Returns integer width, depending on format
  widthToInt(val) {
    if (this.advanceWidthFormat === 0) return Math.round(val);

    return Math.round(val * 16);
  }

  // Convert kerning to FP4.4, useable for writer. Apply `kerningScale`.
  kernToFP(val) {
    return Math.round(val / this.kerningScale * 16);
  }

  toBin() {
    const result = Buffer.concat([
      this.head.toBin(),
      this.cmap.toBin(),
      this.loca.toBin(),
      this.glyf.toBin(),
      this.kern.toBin()
    ]);

    debug(`font size: ${result.length}`);

    return result;
  }
}


module.exports = Font;
