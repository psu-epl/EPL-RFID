/******************************************************************
  * Copyright (c) 2018 Daniel Eynis, Bishoy Hanna, Bryan Mikkelson,
  * Justin Moore, Huy Nguyen, Michael Olivas, Andrew Wood          
  * This program is licensed under the "MIT License".              
  * Please see the file LICENSE in the source                      
  * distribution of this software for license terms.               
/******************************************************************/

'use strict';

var fs = require('fs');
var path = require('path');
var Sequelize = require('sequelize');
var basename = path.basename(__filename);
var config = require(__dirname + '/../../local_modules/config');
var user = config.get('Database', 'username');
var database = config.get('Database', 'database');
var password = config.get('Database', 'password');
var loggingSetting = config.get('Database', 'logging');

if(loggingSetting === 'false')
  loggingSetting = false;
else
  loggingSetting = true;

var configurations = {
  host: config.get('Database', 'host'),
  dialect: config.get('Database', 'dialect'),
  port: config.get('Database', 'port'),
  logging: loggingSetting
};
var db = {};
var sequelize = new Sequelize(database, user, password, configurations);

fs
    .readdirSync(__dirname)
    .filter(file => {
  return (file.indexOf('.') !== 0) && (file !== basename) && (file.slice(-3) === '.js');
})
.forEach(file => {
  var model = sequelize['import'](path.join(__dirname, file));
db[model.name] = model;
});

Object.keys(db).forEach(modelName => {
  if (db[modelName].associate) {
  db[modelName].associate(db);
}
});

db.sequelize = sequelize;
db.Sequelize = Sequelize;

module.exports = db;
