'use strict';


const u = require('../utils');
const debug = require('debug')('font.table.head');

const O_SIZE = 0;
const O_LABEL = O_SIZE + 4;
const O_VERSION = O_LABEL + 4;
const O_TABLES = O_VERSION + 4;
const O_FONT_SIZE = O_TABLES + 2;
const O_ASCENT = O_FONT_SIZE + 2;
const O_DESCENT = O_ASCENT + 2;
const O_TYPO_ASCENT = O_DESCENT + 2;
const O_TYPO_DESCENT = O_TYPO_ASCENT + 2;
const O_TYPO_LINE_GAP = O_TYPO_DESCENT + 2;
const O_MIN_Y = O_TYPO_LINE_GAP + 2;
const O_MAX_Y = O_MIN_Y + 2;
const O_DEF_ADVANCE_WIDTH = O_MAX_Y + 2;
const O_KERNING_SCALE = O_DEF_ADVANCE_WIDTH + 2;
const O_INDEX_TO_LOC_FORMAT = O_KERNING_SCALE + 2;
const O_GLYPH_ID_FORMAT = O_INDEX_TO_LOC_FORMAT + 1;
const O_ADVANCE_WIDTH_FORMAT = O_GLYPH_ID_FORMAT + 1;
const O_BITS_PER_PIXEL = O_ADVANCE_WIDTH_FORMAT + 1;
const O_XY_BITS = O_BITS_PER_PIXEL + 1;
const O_WH_BITS = O_XY_BITS + 1;
const O_ADVANCE_WIDTH_BITS = O_WH_BITS + 1;
const O_COMPRESSION_ID = O_ADVANCE_WIDTH_BITS + 1;
const O_SUBPIXELS_MODE = O_COMPRESSION_ID + 1;
const O_TMP_RESERVED1 = O_SUBPIXELS_MODE + 1;
const O_UNDERLINE_POSITION = O_TMP_RESERVED1 + 1;
const O_UNDERLINE_THICKNESS = O_UNDERLINE_POSITION + 2;
const HEAD_LENGTH = u.align4(O_UNDERLINE_THICKNESS + 2);


class Head {
  constructor(font) {
    this.font = font;
    this.label = 'head';
    this.version = 1;
  }

  toBin() {
    const buf = Buffer.alloc(HEAD_LENGTH);
    debug(`table size = ${buf.length}`);

    buf.writeUInt32LE(HEAD_LENGTH, O_SIZE);
    buf.write(this.label, O_LABEL);
    buf.writeUInt32LE(this.version, O_VERSION);

    const f = this.font;

    const tables_count = f.hasKerning() ? 4 : 3;

    buf.writeUInt16LE(tables_count, O_TABLES);

    buf.writeUInt16LE(f.src.size, O_FONT_SIZE);
    buf.writeUInt16LE(f.src.ascent, O_ASCENT);
    buf.writeInt16LE(f.src.descent, O_DESCENT);

    buf.writeUInt16LE(f.src.typoAscent, O_TYPO_ASCENT);
    buf.writeInt16LE(f.src.typoDescent, O_TYPO_DESCENT);
    buf.writeUInt16LE(f.src.typoLineGap, O_TYPO_LINE_GAP);

    buf.writeInt16LE(f.minY, O_MIN_Y);
    buf.writeInt16LE(f.maxY, O_MAX_Y);

    if (f.monospaced) {
      buf.writeUInt16LE(f.widthToInt(f.src.glyphs[0].advanceWidth), O_DEF_ADVANCE_WIDTH);
    } else {
      buf.writeUInt16LE(0, O_DEF_ADVANCE_WIDTH);
    }

    buf.writeUInt16LE(Math.round(f.kerningScale * 16), O_KERNING_SCALE); // FP12.4

    buf.writeUInt8(f.indexToLocFormat, O_INDEX_TO_LOC_FORMAT);
    buf.writeUInt8(f.glyphIdFormat, O_GLYPH_ID_FORMAT);
    buf.writeUInt8(f.advanceWidthFormat, O_ADVANCE_WIDTH_FORMAT);

    buf.writeUInt8(f.opts.bpp, O_BITS_PER_PIXEL);
    buf.writeUInt8(f.xy_bits, O_XY_BITS);
    buf.writeUInt8(f.wh_bits, O_WH_BITS);

    if (f.monospaced) buf.writeUInt8(0, O_ADVANCE_WIDTH_BITS);
    else buf.writeUInt8(f.advanceWidthBits, O_ADVANCE_WIDTH_BITS);

    buf.writeUInt8(f.glyf.getCompressionCode(), O_COMPRESSION_ID);

    buf.writeUInt8(f.subpixels_mode, O_SUBPIXELS_MODE);

    buf.writeInt16LE(f.src.underlinePosition, O_UNDERLINE_POSITION);
    buf.writeUInt16LE(f.src.underlineThickness, O_UNDERLINE_POSITION);

    return buf;
  }
}


module.exports = Head;
