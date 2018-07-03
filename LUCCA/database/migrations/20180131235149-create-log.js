/******************************************************************
  * Copyright (c) 2018 Daniel Eynis, Bishoy Hanna, Bryan Mikkelson,
  * Justin Moore, Huy Nguyen, Michael Olivas, Andrew Wood          
  * This program is licensed under the "MIT License".              
  * Please see the file LICENSE in the source                      
  * distribution of this software for license terms.               
/******************************************************************/

'use strict';
module.exports = {
  up: (queryInterface, Sequelize) => {
    return queryInterface.createTable('logs', {
      id: {
        autoIncrement: true,
        primaryKey: true,
        type: Sequelize.INTEGER
      },
      eventClass: {
        allowNull: false,
        type: Sequelize.STRING
      },
      entity:
      {
        allowNull: false,
        type: Sequelize.STRING,
        defaultValue: "n/a"
      },
      event: {
        allowNull: false,
        type: Sequelize.STRING(4096)
      },
      eventDate: {
        allowNull: false,
        type: Sequelize.DATE,
        defaultValue: Sequelize.fn('now')
      },
    }, {timestamps:false});
  },
  down: (queryInterface, Sequelize) => {
    return queryInterface.dropTable('logs');
  }
};
