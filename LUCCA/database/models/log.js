/******************************************************************
  * Copyright (c) 2018 Daniel Eynis, Bishoy Hanna, Bryan Mikkelson,
  * Justin Moore, Huy Nguyen, Michael Olivas, Andrew Wood          
  * This program is licensed under the "MIT License".              
  * Please see the file LICENSE in the source                      
  * distribution of this software for license terms.               
/******************************************************************/

'use strict';
module.exports = (sequelize, DataTypes) => {
  var log = sequelize.define('log', {
    id:
      {
        type: DataTypes.INTEGER,
        autoIncrement: true,
        primaryKey: true
      },
    eventClass:
      {
        type: DataTypes.STRING,
        allowNull: false
      },
    entity:
      {
        type: DataTypes.STRING,
        allowNull: false,
        defaultValue: "n/a"
      },
    event:
      {
        type: DataTypes.STRING(4096),
        defaultValue: null
      },
    eventDate:
      {
        allowNull: false,
        type: DataTypes.DATE,
        defaultValue: sequelize.fn('now')
      },
  }, {timestamps: false});
  return log;
};
