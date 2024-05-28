// Parse input arguments and execute convertor

'use strict';


const argparse    = require('argparse');
const fs          = require('fs');
const mkdirp      = require('mkdirp');
const path        = require('path');
const convert     = require('./convert');


class ActionFontAdd extends argparse.Action {
  call(parser, namespace, value/*, option_string*/) {
    let items = (namespace[this.dest] || []).slice();
    items.push({ source_path: value, ranges: [] });
    namespace[this.dest] = items;
  }
}


// add range or symbols to font;
// need to merge them into one array here so overrides work correctly
class ActionFontRangeAdd extends argparse.Action {
  call(parser, namespace, value, option_string) {
    let fonts = namespace.font || [];

    if (fonts.length === 0) {
      parser.error(`argument ${option_string}: Only allowed after --font`);
    }

    let lastFont = fonts[fonts.length - 1];

    // { symbols: 'ABC' }, or { range: [ 65, 67, 65 ] }
    lastFont.ranges.push({ [this.dest]: value });
  }
}


// add hinting option to font;
class ActionFontStoreTrue extends argparse.Action {
  constructor(options) {
    options = options || {};
    options.const = true;
    options.default = options.default !== null ? options.default : false;
    options.nargs = 0;
    super(options);
  }

  call(parser, namespace, value, option_string) {
    let fonts = namespace.font || [];

    if (fonts.length === 0) {
      parser.error(`argument ${option_string}: Only allowed after --font`);
    }

    let lastFont = fonts[fonts.length - 1];

    lastFont[this.dest] = this.const;
  }
}


// Formatter with support of `\n` in Help texts.
class RawTextHelpFormatter2 extends argparse.RawDescriptionHelpFormatter {
  // executes parent _split_lines for each line of the help, then flattens the result
  _split_lines(text, width) {
    return [].concat(...text.split('\n').map(line => super._split_lines(line, width)));
  }
}


// parse decimal or hex code in unicode range
function unicode_point(str) {
  let m = /^(?:(?:0x([0-9a-f]+))|([0-9]+))$/i.exec(str.trim());

  if (!m) throw new TypeError(`${str} is not a number`);

  let [ , hex, dec ] = m;

  let value = hex ? parseInt(hex, 16) : parseInt(dec, 10);

  if (value > 0x10FFFF) throw new TypeError(`${str} is out of unicode range`);

  return value;
}


// parse range
function range(str) {
  let result = [];

  for (let s of str.split(',')) {
    let m = /^(.+?)(?:-(.+?))?(?:=>(.+?))?$/i.exec(s);

    let [ , start, end, mapped_start ] = m;

    if (!end) end = start;
    if (!mapped_start) mapped_start = start;

    start = unicode_point(start);
    end = unicode_point(end);

    if (start > end) throw new TypeError(`Invalid range: ${s}`);

    mapped_start = unicode_point(mapped_start);

    result.push(start, end, mapped_start);
  }

  return result;
}


// exclude negative numbers and non-numbers
function positive_int(str) {
  if (!/^\d+$/.test(str)) throw new TypeError(`${str} is not a valid number`);

  let n = parseInt(str, 10);

  if (n <= 0) throw new TypeError(`${str} is not a valid number`);

  return n;
}


module.exports.run = async function (argv, debug = false) {

  //
  // Configure CLI
  //

  let parser = new argparse.ArgumentParser({
    add_help: true,
    formatter_class: RawTextHelpFormatter2
  });

  if (debug) {
    parser.exit = function (status, message) {
      throw new Error(message);
    };
  }

  parser.add_argument('-v', '--version', {
    action: 'version',
    version: require('../package.json').version
  });

  parser.add_argument('--size', {
    metavar: 'PIXELS',
    type: positive_int,
    required: true,
    help: 'Output font size, pixels.'
  });

  parser.add_argument('-o', '--output', {
    metavar: '<path>',
    help: 'Output path.'
  });

  parser.add_argument('--bpp', {
    choices: [ 1, 2, 3, 4, 8 ],
    type: positive_int,
    required: true,
    help: 'Bits per pixel, for antialiasing.'
  });

  let lcd_group = parser.add_mutually_exclusive_group();

  lcd_group.add_argument('--lcd', {
    action: 'store_true',
    default: false,
    help: 'Enable subpixel rendering (horizontal pixel layout).'
  });

  lcd_group.add_argument('--lcd-v', {
    action: 'store_true',
    default: false,
    help: 'Enable subpixel rendering (vertical pixel layout).'
  });

  parser.add_argument('--use-color-info', {
    dest: 'use_color_info',
    action: 'store_true',
    default: false,
    help: 'Try to use glyph color info from font to create grayscale icons. ' +
          'Since gray tones are emulated via transparency, result will be good on contrast background only.'
  });

  parser.add_argument('--format', {
    choices: convert.formats,
    required: true,
    help: 'Output format.'
  });

  parser.add_argument('--font', {
    metavar: '<path>',
    action: ActionFontAdd,
    required: true,
    help: 'Source font path. Can be used multiple times to merge glyphs from different fonts.'
  });

  parser.add_argument('-r', '--range', {
    type: range,
    action: ActionFontRangeAdd,
    help: `
Range of glyphs to copy. Can be used multiple times, belongs to previously declared "--font". Examples:
  -r 0x1F450
  -r 0x20-0x7F
  -r 32-127
  -r 32-127,0x1F450
  -r '0x1F450=>0xF005'
  -r '0x1F450-0x1F470=>0xF005'
`
  });

  parser.add_argument('--symbols', {
    action: ActionFontRangeAdd,
    help: `
List of characters to copy, belongs to previously declared "--font". Examples:
  --symbols ,.0123456789
  --symbols abcdefghigklmnopqrstuvwxyz
`
  });

  parser.add_argument('--autohint-off', {
    type: range,
    action: ActionFontStoreTrue,
    help: 'Disable autohinting for previously declared "--font"'
  });

  parser.add_argument('--autohint-strong', {
    type: range,
    action: ActionFontStoreTrue,
    help: 'Use more strong autohinting for previously declared "--font" (will break kerning)'
  });

  parser.add_argument('--force-fast-kern-format', {
    dest: 'fast_kerning',
    action: 'store_true',
    default: false,
    help: 'Always use kern classes instead of pairs (might be larger but faster).'
  });

  parser.add_argument('--no-compress', {
    dest: 'no_compress',
    action: 'store_true',
    default: false,
    help: 'Disable built-in RLE compression.'
  });

  parser.add_argument('--no-prefilter', {
    dest: 'no_prefilter',
    action: 'store_true',
    default: false,
    help: 'Disable bitmap lines filter (XOR), used to improve compression ratio.'
  });

  parser.add_argument('--no-kerning', {
    dest: 'no_kerning',
    action: 'store_true',
    default: false,
    help: 'Drop kerning info to reduce size (not recommended).'
  });

  parser.add_argument('--lv-include', {
    metavar: '<path>',
    help: 'Set alternate "lvgl.h" path (for --format lvgl).'
  });

  parser.add_argument('--full-info', {
    dest: 'full_info',
    action: 'store_true',
    default: false,
    help: 'Don\'t shorten "font_info.json" (include pixels data).'
  });

  //
  // Process CLI options
  //

  let args = parser.parse_args(argv.length ? argv : [ '-h' ]);

  for (let font of args.font) {
    if (font.ranges.length === 0) {
      parser.error(`You need to specify either "--range" or "--symbols" for font "${font.source_path}"`);
    }

    try {
      font.source_bin = fs.readFileSync(font.source_path);
    } catch (err) {
      parser.error(`Cannot read file "${font.source_path}": ${err.message}`);
    }
  }

  //
  // Convert
  //

  let files = await convert(args);

  //
  // Store files
  //

  for (let [ filename, data ] of Object.entries(files)) {
    let dir = path.dirname(filename);

    mkdirp.sync(dir);

    fs.writeFileSync(filename, data);
  }

};


// export for tests
module.exports._range = range;
