// Write font data into png images

'use strict';


const path     = require('path');
const { PNG }  = require('pngjs');
const AppError = require('../app_error');
const utils    = require('../utils');

const normal_color  = [ 255, 255, 255 ];
const outside_color = [ 255, 127, 184 ];


module.exports = function write_images(args, font) {
  if (!args.output) throw new AppError('Output is required for "dump" writer');

  let files = {};

  let glyphs = font.glyphs.map(glyph => utils.set_depth(glyph, args.bpp));

  for (let glyph of glyphs) {
    let { code, advanceWidth, bbox, pixels } = glyph;

    advanceWidth = Math.round(advanceWidth);

    let minX = bbox.x;
    let maxX = Math.max(bbox.x + bbox.width - 1, bbox.x);
    let minY = Math.min(bbox.y, font.typoDescent);
    let maxY = Math.max(bbox.y + bbox.height - 1, font.typoAscent);

    let png = new PNG({ width: maxX - minX + 1, height: maxY - minY + 1 });

    /* eslint-disable max-depth */
    for (let pos = 0, y = maxY; y >= minY; y--) {
      for (let x = minX; x <= maxX; x++) {
        let value = 0;

        if (x >= bbox.x && x < bbox.x + bbox.width && y >= bbox.y && y < bbox.y + bbox.height) {
          value = pixels[bbox.height - (y - bbox.y) - 1][x - bbox.x];
        }

        let r, g, b;

        if (x < 0 || x >= advanceWidth || y < font.typoDescent || y > font.typoAscent) {
          [ r, g, b ] = outside_color;
        } else {
          [ r, g, b ] = normal_color;
        }

        png.data[pos++] = (255 - value) * r / 255;
        png.data[pos++] = (255 - value) * g / 255;
        png.data[pos++] = (255 - value) * b / 255;
        png.data[pos++] = 255;
      }
    }


    files[path.join(args.output, `${code.toString(16)}.png`)]  = PNG.sync.write(png);
  }

  files[path.join(args.output, 'font_info.json')] = JSON.stringify(
    font,
    (k, v) => (k === 'pixels' && !args.full_info ? undefined : v),
    2);

  return files;
};
