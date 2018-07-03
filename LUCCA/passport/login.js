/******************************************************************
  * Copyright (c) 2018 Daniel Eynis, Bishoy Hanna, Bryan Mikkelson,
  * Justin Moore, Huy Nguyen, Michael Olivas, Andrew Wood          
  * This program is licensed under the "MIT License".              
  * Please see the file LICENSE in the source                      
  * distribution of this software for license terms.               
/******************************************************************/

var LocalStrategy = require('passport-local').Strategy;
var bCrypt = require('bcrypt-nodejs');
var db = require('../database/controllers/dbm');

var validPassword = function (user_pass, password) {
  return bCrypt.compareSync(password, user_pass);
}

module.exports = new LocalStrategy({
  passReqToCallback: true,
  usernameField: 'email',
  passwordField: 'password'
},
  function (req, username, password, done) {
    db.validateUser(username).then(function (user) {
      if (user === undefined) {
        req.flash('error', 'Email or password is incorrect');
        return done(null, false);
      }

      if (user.password === null || !validPassword(user.password, password)) {
        req.flash('error', 'Email or password is incorrect');
        return done(null, false);
      }

      let adminName = user.first + ' ' + user.last;
      db.logEvent('administration', user.badge, `Admin ${adminName} (${user.email}) has logged into the web app through UI`);

      return done(null, user);
    });
  }
);