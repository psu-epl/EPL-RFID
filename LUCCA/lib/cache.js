/******************************************************************
  * Copyright (c) 2018 Daniel Eynis, Bishoy Hanna, Bryan Mikkelson,
  * Justin Moore, Huy Nguyen, Michael Olivas, Andrew Wood          
  * This program is licensed under the "MIT License".              
  * Please see the file LICENSE in the source                      
  * distribution of this software for license terms.               
/******************************************************************/

"use strict";

//Private object
var _cache = Object.create(null);

module.exports = {

	//add machine to cache with user 
	//waiting for privilages 
	save: (userToAdd, UserBadge, machine) => {
		var record = {
			user: userToAdd,
			badge: UserBadge,
			start: new Date()
		};
		_cache[machine] = record;
		return _cache;
	},

	// return user that is cached with machine
	getMachine: (machine, time) => {
		var data = _cache[machine];
		if (typeof data != 'undefined') {
			
			let currentTime = new Date()
			let dif = Math.abs(currentTime - data.start) / 1000;
			
			if(dif  <= time) {
				return data;
			} else {
					delete _cache[machine];
					return null;
			};
		};
		return null;
	},

	//delete from cache
	delete: (machine) => {
		delete _cache[machine];
	}

};