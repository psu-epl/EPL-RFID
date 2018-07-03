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
  return queryInterface.bulkInsert('users', [{
    badge: '00000000000000',
    first: 'admin',
    last: 'admin',
    email: 'admin@admin.com',
    phone: '555-555-5555',
    signature: 'admin',
    ecSignature: 'admin',
    ecName: 'admin',
    ecRel: 'admin',
    ecPhone: '555-444-5555',
    status: 'Admin',
    password: '$2a$10$aW/Ed4Wzpz9Vg4oJKYpLh.HOrqwWO8a7f0nGm.TNL9Lbz1N4xPpBy',
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
