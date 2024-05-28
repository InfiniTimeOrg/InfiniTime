// Write font in lvgl format
'use strict';


const AppError = require('../../app_error');
const Font     = require('./lv_font');


module.exports = function write_images(args, fontData) {
  if (!args.output) throw new AppError('Output is required for "lvgl" writer');

  const font = new Font(fontData, args);

  return {
    [args.output]: font.toLVGL()
  };
};
