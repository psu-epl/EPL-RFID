/******************************************************************
  * Copyright (c) 2018 Daniel Eynis, Bishoy Hanna, Bryan Mikkelson,
  * Justin Moore, Huy Nguyen, Michael Olivas, Andrew Wood          
  * This program is licensed under the "MIT License".              
  * Please see the file LICENSE in the source                      
  * distribution of this software for license terms.               
/******************************************************************/

"use strict";

//Private object
var _stationObj = Object.create(null);

module.exports = {

	addMachine: (machine) => {
		if(!machine || _stationObj[machine]) {
			return null;
		};
		var station = {
			badge: null,
			user: null,
			status: 'disabled'
		};
		_stationObj[machine] = station;
		return _stationObj[machine];
	},

	
	getMachine: (machine) => {
		if (typeof _stationObj[machine] != 'undefined') {
			return _stationObj[machine];
		};
		return null
	},


	addUser: (userToAdd, userBadge, machine) => {
		if(typeof _stationObj[machine] != 'undefined' && userToAdd) {
			_stationObj[machine].user = userToAdd;
			_stationObj[machine].badge = userBadge;
			_stationObj[machine].status = 'enabled'
			return _stationObj[machine];
		};	
		return false;
  },
  
  
  getAll: () => {
    return _stationObj;
  },
	

	removeUser: (userBadge, machine) => {
		if(typeof _stationObj[machine] != 'undefined' && userBadge) {
			if(_stationObj[machine].badge === userBadge) {
				_stationObj[machine].badge = null;
				_stationObj[machine].user = null;
				_stationObj[machine].status = 'disabled';
				return _stationObj[machine];
			} else {
					return null;
			}
		};
		return null
	},

	
	updateMachine: (machine, setState) => {
		if(machine){
			if(_stationObj[machine]){
				_stationObj[machine].badge = null;
				_stationObj[machine].user = null;
				_stationObj[machine].status = setState;
			}else{
				module.exports.addMachine(machine);
			}
			return true;
		}
		return false;
 
 },

};