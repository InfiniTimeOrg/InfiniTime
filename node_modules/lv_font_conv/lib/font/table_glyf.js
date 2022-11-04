'use strict';

const u = require('../utils');
const { BitStream } = require('bit-buffer');
const debug = require('debug')('font.table.glyf');
const compress = require('./compress');


const O_SIZE = 0;
const O_LABEL = O_SIZE + 4;

const HEAD_LENGTH = O_LABEL + 4;


class Glyf {
  constructor(font) {
    this.font = font;
    this.label = 'glyf';

    this.compiled = false;

    this.binData = [];
  }

  // convert 8-bit opacity to bpp-bit
  pixelsToBpp(pixels) {
    const bpp = this.font.opts.bpp;
    return pixels.map(line => line.map(p => (p >>> (8 - bpp))));
  }

  // Returns "binary stream" (Buffer) of compiled glyph data
  compileGlyph(glyph) {
    // Allocate memory, enough for eny storage formats
    const buf = Buffer.alloc(100 + glyph.bbox.width * glyph.bbox.height * 4);
    const bs = new BitStream(buf);
    bs.bigEndian = true;
    const f = this.font;

    // Store Width
    if (!f.monospaced) {
      let w = f.widthToInt(glyph.advanceWidth);
      bs.writeBits(w, f.advanceWidthBits);
    }

    // Store X, Y
    bs.writeBits(glyph.bbox.x, f.xy_bits);
    bs.writeBits(glyph.bbox.y, f.xy_bits);
    bs.writeBits(glyph.bbox.width, f.wh_bits);
    bs.writeBits(glyph.bbox.height, f.wh_bits);

    const pixels = this.pixelsToBpp(glyph.pixels);

    this.storePixels(bs, pixels);

    // Shrink size
    const result = Buffer.alloc(bs.byteIndex);
    buf.copy(result, 0, 0, bs.byteIndex);

    return result;
  }

  storePixels(bitStream, pixels) {
    if (this.getCompressionCode() === 0) this.storePixelsRaw(bitStream, pixels);
    else this.storePixelsCompressed(bitStream, pixels);
  }

  storePixelsRaw(bitStream, pixels) {
    const bpp = this.font.opts.bpp;

    for (let y = 0; y < pixels.length; y++) {
      const line = pixels[y];
      for (let x = 0; x < line.length; x++) {
        bitStream.writeBits(line[x], bpp);
      }
    }
  }

  storePixelsCompressed(bitStream, pixels) {
    let p;

    if (this.font.opts.no_prefilter) p = pixels.flat();
    else p = u.prefilter(pixels).flat();

    compress(bitStream, p, this.font.opts);
  }

  // Create internal struct with binary data for each glyph
  // Needed to calculate offsets & build final result
  compile() {
    this.compiled = true;

    this.binData = [
      Buffer.alloc(0) // Reserve id 0
    ];

    const f = this.font;

    f.src.glyphs.forEach(g => {
      const id = f.glyph_id[g.code];

      this.binData[id] = this.compileGlyph(g);
    });
  }

  toBin() {
    if (!this.compiled) this.compile();

    const buf = u.balign4(Buffer.concat([
      Buffer.alloc(HEAD_LENGTH),
      Buffer.concat(this.binData)
    ]));

    buf.writeUInt32LE(buf.length, O_SIZE);
    buf.write(this.label, O_LABEL);

    debug(`table size = ${buf.length}`);

    return buf;
  }

  getSize() {
    if (!this.compiled) this.compile();

    return u.align4(HEAD_LENGTH + u.sum(this.binData.map(b => b.length)));
  }

  getOffset(id) {
    if (!this.compiled) this.compile();

    let offset = HEAD_LENGTH;

    for (let i = 0; i < id; i++) offset += this.binData[i].length;

    return offset;
  }

  getCompressionCode() {
    if (this.font.opts.no_compress) return 0;
    if (this.font.opts.bpp === 1) return 0;

    if (this.font.opts.no_prefilter) return 2;
    return 1;
  }
}


module.exports = Glyf;
