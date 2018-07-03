/******************************************************************
  * Copyright (c) 2018 Daniel Eynis, Bishoy Hanna, Bryan Mikkelson,
  * Justin Moore, Huy Nguyen, Michael Olivas, Andrew Wood          
  * This program is licensed under the "MIT License".              
  * Please see the file LICENSE in the source                      
  * distribution of this software for license terms.               
/******************************************************************/

// fetch server runtime parameters from conf file
var ConfigParser = require('configparser');

var conf_name = 'lucca.conf';

const config = new ConfigParser();
config.read(conf_name);

config._write = config.write;
config.write = function(){
  this._write(conf_name);
}

module.exports = config;

// The most relevant API features exposed by this wrapper module are:
//
// config.sections()
//   - returns a list of strings containing the names of all sections defined
//     in the config file.
//
// config.get('Section_Name', 'parameter_ame')
//   - returns a string containing the value of the requested parameter in the
//     given section, if it exists (or undefined it it does not))
//
// config.set('Section_Name', 'parameter_name', 'value')
//   - alters the in-memory cached copy of the config to set the specified
//     parameter to the given value. This change is application-wide.
//
// config.write()
//   - flushes the in-memory copy of the config, overwriting the config file
//     with any changes that have been made with .set() and .addSection()
//
// see https://github.com/ZachPerkitny/configparser for "full documentation"
