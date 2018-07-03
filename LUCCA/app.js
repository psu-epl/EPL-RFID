/******************************************************************
  * Copyright (c) 2018 Daniel Eynis, Bishoy Hanna, Bryan Mikkelson,
  * Justin Moore, Huy Nguyen, Michael Olivas, Andrew Wood          
  * This program is licensed under the "MIT License".              
  * Please see the file LICENSE in the source                      
  * distribution of this software for license terms.               
/******************************************************************/

var express = require('express');
var path = require('path'); 
var favicon = require('serve-favicon');
var logger = require('morgan');
var cookieParser = require('cookie-parser');
var bodyParser = require('body-parser');
var nunjucks = require('nunjucks');
var passport = require('passport');
var session = require('express-session');

var config = require('./local_modules/config');
var login_strategy = require('./passport/login.js');
var register_strategy = require('./passport/register.js');
var reset_strategy = require('./passport/reset.js');
var db = require('./database/controllers/dbm');
var flash = require('express-flash');

var index = require('./routes/index');
var api = require('./routes/api');

var app = express();

// view engine setup
nunjucks.configure('views', {
  autoescape: true,
  express: app
});

app.set('view engine', 'nunjucks');

app.use(favicon(path.join(__dirname, 'public', 'favicon.ico')));
app.use(logger('dev'));
app.use(cookieParser('my_secret'));
app.use(session({ secret: 'my_secret_session_key', cookie: { maxAge: 3600000 } }));
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));
app.use(passport.initialize());
app.use(passport.session());
app.use(express.static(path.join(__dirname, 'public')));
app.use(flash());

app.use('/', require('./routes/index')(passport));
app.use('/api', api);

passport.serializeUser(function (user, done) {
  done(null, user.badge);
});

passport.deserializeUser(function (id, done) {
  db.validateUser(id).then(function (user) {
    done(null, user);
  });
});

passport.use('login', login_strategy);
passport.use('register', register_strategy);
passport.use('reset', reset_strategy);

// catch 404 and forward to error handler
app.use(function (req, res, next) {
  var err = new Error('Not Found');
  err.status = 404;
  next(err);
});

// error handler
app.use(function (err, req, res, next) {
  // set locals, only providing error in development
  res.locals.message = err.message;
  res.locals.error = req.app.get('env') === 'development' ? err : {};

  // render the error page
  res.status(err.status || 500);
  res.render('error');
});

module.exports = app;
