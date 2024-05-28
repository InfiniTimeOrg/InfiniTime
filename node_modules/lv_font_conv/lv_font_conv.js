#!/usr/bin/env node

'use strict';

const AppError = require('./lib/app_error');

require('./lib/cli').run(process.argv.slice(2)).catch(err => {
  /*eslint-disable no-console*/
  if (err instanceof AppError) {
    // Try to beautify normal errors
    console.error(err.message.trim());
  } else {
    // Print crashes
    console.error(err.stack);
  }
  process.exit(1);
});
