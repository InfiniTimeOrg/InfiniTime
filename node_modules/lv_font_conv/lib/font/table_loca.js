'use strict';


const u = require('../utils');
const debug = require('debug')('font.table.loca');


const O_SIZE = 0;
const O_LABEL = O_SIZE + 4;
const O_COUNT = O_LABEL + 4;

const HEAD_LENGTH = O_COUNT + 4;


class Loca {
  constructor(font) {
    this.font = font;
    this.label = 'loca';
  }

  toBin() {
    const f = this.font;

    const offsets = [ ...Array(f.last_id).keys() ].map(i => f.glyf.getOffset(i));

    const buf = u.balign4(Buffer.concat([
      Buffer.alloc(HEAD_LENGTH),
      f.indexToLocFormat ? u.bFromA32(offsets) : u.bFromA16(offsets)
    ]));

    buf.writeUInt32LE(buf.length, O_SIZE);
    buf.write(this.label, O_LABEL);
    buf.writeUInt32LE(f.last_id, O_COUNT);

    debug(`table size = ${buf.length}`);

    return buf;
  }
}


module.exports = Loca;
