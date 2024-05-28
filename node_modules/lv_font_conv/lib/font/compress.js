'use strict';

//const debug = require('debug')('compress');

function count_same(arr, offset) {
  let same = 1;
  let val = arr[offset];

  for (let i = offset + 1; i < arr.length; i++) {
    if (arr[i] !== val) break;
    same++;
  }

  return same;
}

//
// Compress pixels with RLE-like algorythm (modified I3BN)
//
// 1. Require minimal repeat count (1) to enter I3BN mode
// 2. Increased 1-bit-replaced repeat limit (2 => 10)
// 3. Length of direct repetition counter reduced (8 => 6 bits).
//
// pixels - flat array of pixels (one per entry)
// options.bpp - bits per pixels
//
module.exports = function compress(bitStream, pixels, options) {
  const opts = Object.assign({}, { repeat: 1 }, options);

  // Minimal repetitions count to enable RLE mode.
  const RLE_SKIP_COUNT = 1;
  // Number of repeats, when `1` used to replace data
  // If more - write as number
  const RLE_BIT_COLLAPSED_COUNT = 10;

  const RLE_COUNTER_BITS = 6; // (2^bits - 1) - max value
  const RLE_COUNTER_MAX = (1 << RLE_COUNTER_BITS) - 1;
  // Force flush if counter dencity exceeded.
  const RLE_MAX_REPEATS = RLE_COUNTER_MAX + RLE_BIT_COLLAPSED_COUNT + 1;

  //let bits_start_offset = bitStream.index;

  let offset = 0;

  while (offset < pixels.length) {
    const p = pixels[offset];

    let same = count_same(pixels, offset);

    // Clamp value because RLE counter density is limited
    if (same > RLE_MAX_REPEATS + RLE_SKIP_COUNT) {
      same = RLE_MAX_REPEATS + RLE_SKIP_COUNT;
    }

    //debug(`offset: ${offset}, count: ${same}, pixel: ${p}`);

    offset += same;

    // If not enough for RLE - write as is.
    if (same <= RLE_SKIP_COUNT) {
      for (let i = 0; i < same; i++) {
        bitStream.writeBits(p, opts.bpp);
        //debug(`==> ${opts.bpp} bits`);
      }
      continue;
    }

    // First, write "skipped" head as is.
    for (let i = 0; i < RLE_SKIP_COUNT; i++) {
      bitStream.writeBits(p, opts.bpp);
      //debug(`==> ${opts.bpp} bits`);
    }

    same -= RLE_SKIP_COUNT;

    // Not reached state to use counter => dump bit-extended
    if (same <= RLE_BIT_COLLAPSED_COUNT) {
      bitStream.writeBits(p, opts.bpp);
      //debug(`==> ${opts.bpp} bits (val)`);
      for (let i = 0; i < same; i++) {
        /*eslint-disable max-depth*/
        if (i < same - 1) {
          bitStream.writeBits(1, 1);
          //debug('==> 1 bit (rle repeat)');
        } else {
          bitStream.writeBits(0, 1);
          //debug('==> 1 bit (rle repeat last)');
        }
      }
      continue;
    }

    same -= RLE_BIT_COLLAPSED_COUNT + 1;

    bitStream.writeBits(p, opts.bpp);
    //debug(`==> ${opts.bpp} bits (val)`);

    for (let i = 0; i < RLE_BIT_COLLAPSED_COUNT + 1; i++) {
      bitStream.writeBits(1, 1);
      //debug('==> 1 bit (rle repeat)');
    }
    bitStream.writeBits(same, RLE_COUNTER_BITS);
    //debug(`==> 4 bits (rle repeat count ${same})`);
  }

  //debug(`output bits: ${bitStream.index - bits_start_offset}`);
};
