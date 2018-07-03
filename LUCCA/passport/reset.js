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


module.exports = new LocalStrategy({
  passReqToCallback: true,
  usernameField: 'email',
  passwordField: 'password'
},
  function (req, username, password, done) {
    db.validateUser(username).then(function (user) {
      if (user === undefined) {
        req.flash('error', 'User does not already exist in database, please register user first');
        return done(null, false);
      }

      if (user.status !== 'Admin') {
        req.flash('error', 'Can only reset account which are already admin!');
        return done(null, false);
      }

      let adminName = req.user.first + ' ' + req.user.last;
      let adminEmail = req.user.email;
      let adminBadge = req.user.badge;
      db.logEvent('administration', adminBadge, `Admin ${adminName} (${adminEmail}) has reset the admin password for admin with badge ID: ${user.badge}`);

      var newAdminCreds = {
        password: bCrypt.hashSync(password),
      }

      db.modifyUser(user.badge, newAdminCreds).then(function (result) {
        db.validateUser(user.badge).then(function (newUser) {
          return done(null, newUser);
        });
      });
    });
  }
);