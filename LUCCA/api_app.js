/******************************************************************
  * Copyright (c) 2018 Daniel Eynis, Bishoy Hanna, Bryan Mikkelson,
  * Justin Moore, Huy Nguyen, Michael Olivas, Andrew Wood          
  * This program is licensed under the "MIT License".              
  * Please see the file LICENSE in the source                      
  * distribution of this software for license terms.               
/******************************************************************/

var express = require('express');
var config = require('./local_modules/config');
var router = express.Router({
});

var bodyParser = require('body-parser');

var api = require('./routes/api');

var app = express();
app.use(bodyParser.text());
app.use('/api', api);

app.use(function(req, res, next) {
  var err = new Error('Not Found');
  err.status = 404;
  next(err);
});

// error handler
app.use(function(err, req, res, next) {
  console.log(err);

  res.status(err.status || 500);
  res.send('Internal Server Error');
});

module.exports = app;
