// Custom Error type to simplify error messaging
//
'use strict';


//const ExtendableError = require('es6-error');
//module.exports = class AppError extends ExtendableError {};

module.exports = require('make-error')('AppError');
