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
    return queryInterface.bulkInsert('machines', [{
      sId: '1',
      name: 'Hacksaw',
      description: 'For Cutting Stuff',
      registered: false,
      certCN: 'black-heron.mindtax.net',
      createdAt: new Date(),
      updatedAt: new Date()
    },{
      sId: '2',
      name: '3d Printer',
      //description: 'Guy',
      registered: false,
      certCN: 'localhost',
      createdAt: new Date(),
      updatedAt: new Date()
    },{
      sId: '3',
      name: 'Cannon',
      description: 'Things go Boom!',
      certCN: '*.smithsonian.com',
      createdAt: new Date(),
      updatedAt: new Date()
    },{
      sId: '4',
      name: 'Hacksaw II',
      description: 'For Cutting Stuff Harder',
      registered: true,
      certCN: 'localhost',
      createdAt: new Date(),
      updatedAt: new Date()
    },{
      sId: '5',
      name: '8d Printer',
      //description: 'Guy',
      registered: true,
      certCN: 'localhost',
      createdAt: new Date(),
      updatedAt: new Date()
    },{
      sId: '6',
      name: 'WOOOOOO',
      description: 'Things go Boom!',
      registered: true,
      certCN: 'example.com',
      createdAt: new Date(),
      updatedAt: new Date()
    }], {});
  },

  down: (queryInterface, Sequelize) => {
    /*
      Add reverting commands here.
      Return a promise to correctly handle asynchronicity.

      Example:
      return queryInterface.bulkDelete('Person', null, {});
    */
  }
};
