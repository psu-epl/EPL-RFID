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
      badge: '1234567',
      first: 'Some',
      last: 'Guy',
      email: 'abc@123.com',
      phone: '555-555-5555',
      signature: 'Some Guy',
      ecSignature: 'Dad Person',
      ecName: 'Dad Person',
      ecRel: 'Father',
      ecPhone: '555-444-5555',
      createdAt: new Date(),
      updatedAt: new Date()
    },{
        badge: '8910111',
        first: 'Another',
        last: 'Person',
        email: '123@abc.com',
        phone: '555-444-3333',
        signature: 'Another Person',
        //ecSignature: 'Dad Person',
        ecName: 'Yo Whats Up',
        ecRel: 'Mother',
        ecPhone: '555-444-7777',
        createdAt: new Date(),
        updatedAt: new Date()
    },{
        badge: '2131415',
        first: 'Finally',
        last: 'Done',
        email: 'who@you.com',
        phone: '555-666-7777',
        signature: 'Finally Done',
        //ecSignature: '',
        ecName: 'Whatsa who',
        ecRel: 'Friend of a friend',
        ecPhone: '555-999-1010',
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
