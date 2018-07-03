/******************************************************************
  * Copyright (c) 2018 Daniel Eynis, Bishoy Hanna, Bryan Mikkelson,
  * Justin Moore, Huy Nguyen, Michael Olivas, Andrew Wood          
  * This program is licensed under the "MIT License".              
  * Please see the file LICENSE in the source                      
  * distribution of this software for license terms.               
/******************************************************************/

var config = require(__dirname + '/../../local_modules/config');

module.exports = {
  development: {
    username: config.get('Database', 'username'),
    password: config.get('Database', 'password'),
    database: config.get('Database', 'database'),
    host: config.get('Database', 'host'),
    port: config.get('Database', 'port'),
    dialect: config.get('Database', 'dialect'),
    logging: false
  }
};