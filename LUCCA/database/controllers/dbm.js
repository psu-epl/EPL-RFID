/******************************************************************
  * Copyright (c) 2018 Daniel Eynis, Bishoy Hanna, Bryan Mikkelson,
  * Justin Moore, Huy Nguyen, Michael Olivas, Andrew Wood          
  * This program is licensed under the "MIT License".              
  * Please see the file LICENSE in the source                      
  * distribution of this software for license terms.               
/******************************************************************/

//LUCCA Project - 01-28-2018
//Updated Last  - 02-04-2018
//           By - Bryan Mikkelson
//
//Contributors  -
//                Andy Wood,
//                Bryan Mikkelson,
//                Daniel Eynis
//
"use strict";
const user = require('../models').user;
const machine = require('../models').machine;
const privileges = require('../models').privileges;
const log = require('../models').log;
const db = require('../models');
const Op = db.Sequelize.Op;

/*
    This module will store all the methods for accessing/updating/removing
    user, machine, privileges, and log data in the DB.
 */
module.exports = {

  //Usage: Returns Error Type as String Definitions
  //Arguments:
  //          err - The error that caused the issue
  //          rs  - JSON return object
  //Exceptions: None
  //Descriptions: Multiple Errors can occur when talking
  // to a DB.  This Function will return which type of
  // error it was so the error can be properly logged.
  errorHandling(err, rs) {
    if (err instanceof db.sequelize.ConnectionError) {
      rs.result = false;
      rs.detail = 'Connection Error';
    } else if (err instanceof db.sequelize.DatabaseError) {
      rs.result = false;
      rs.detail = 'Database Error';
    } else if (err instanceof db.sequelize.QueryError) {
      rs.result = false;
      rs.detail = 'Query Error';
    } else if (err instanceof db.sequelize.ValidationError) {
      rs.result = false;
      rs.detail = 'Validation Error';
    } else {
      rs.result = false;
      rs.detail = err;
    }

    return rs;
  },

  //Usage: Returns User data from X-date to Y-date
  //Arguments:
  //          from - Start Date
  //          to   - End Date
  //Exceptions:  ConnectionError, DatabaseError, QueryError,
  // ValidationError, Other
  //Description:  This function will return all User data from
  // the given start date to the given end date.  If start date
  // is undefined then all users will be returned.
  getUsers(from, to) {
    var queryParameters = {};
    var returnStatus = {result: true, detail: 'Success'};
    if (from !== undefined && to !== undefined) {
      queryParameters["createdAt"] = {
        [Op.gte] : from,
        [Op.lte] : to
      }
    } else if (from !== undefined) {
      queryParameters["createdAt"] = {
        [Op.gte] : from
      }
    } else if (to !== undefined) {
      queryParameters["createdAt"] = {
        [Op.lte] : to
      }
    }

    //Return all results
    return user.findAll({
      where: queryParameters,
      raw: true
    }).then(users => {
      return users;
    }).catch(err => {
      return false;
    });
  },

  //Usage: Add a new User to the DB
  //Arguments:
  //         userData - JSON object storing all user data.
  //Exceptions: ConnectionError, DatabaseError, QueryError,
  // ValidationError, Other
  //Description:  This function will take as argument a JSON object
  // with all required user data and then store that data in a backend DB.
  createUser(userData) {
    var returnStatus = { result: true, detail: 'Success' };  //Status Code

    return user.create({
      badge: userData.badge,               //badge#
      first: userData.first,               //first name
      last: userData.last,                 //last name
      email: userData.email,               //email
      phone: userData.phone,               //users phone #
      signature: userData.signature,       //users signature
      ecSignature: userData.ecSignature,   //emergency contact signature
      ecName: userData.ecName,             //emergency contact name
      ecRel: userData.ecRel,               //emergency contact relation
      ecPhone: userData.ecPhone,           //emergency contact phone #
      mailingList: userData.mailingList,   //mailing list sign-up
      createdAt: new Date(),               //date user was added to the DB
      updatedAt: new Date()                //most recent update to users profile
    }).then(function () {
      //New User added Successfully
      return returnStatus;
    }).catch(err => {
      //An Error occurred when trying to add a new user
      return module.exports.errorHandling(err, returnStatus);
    });
  },

  //Usage: Modifies a user's data in the DB
  //Arguments:
  //          bId - User's badge id to identify User
  //          userData - Attribute(s) to be modified
  //Exceptions: ConnectionError, DatabaseError, QueryError,
  // ValidationError, Other
  //Description:  This function will update the desired attributes
  //of a user based of the JSON userData object.  The user will be
  //identified by their bId.
  modifyUser(bId, userData) {
    var returnStatus = { result: true, detail: 'Success' };        //Status Code

    //Update all attributes passed in the userData object where the
    //users badge # = bId
    return user.update(userData, { where: { badge: bId } }).then(results => {

      //If the query executed without error or restriction
      //the results will always be 1, true.  Otherwise,
      //The results are either DB error related or invalid
      //data.
      if (results[0] === 1) {
        return returnStatus;
      } else {
        returnStatus.result = false;
        returnStatus.detail = 'Invalid Data';
        return returnStatus;
      }
    }).catch(err => {
      return module.exports.errorHandling(err, returnStatus);
    });
  },

  //Usage: Determines if a User can or cannot use a station
  //Arguments:
  //          sId - Station Id or undefined
  //          uId - User's identification (badge or email)
  //Exceptions: ConnectionError, DatabaseError, QueryError,
  // ValidationError, Other
  //Description:  This function will tell the user-access API
  // whether or not a user has access to use a machine or not.
  validateUser(uId) {
    if (uId === undefined) {
      return undefined;
    }

    return user.findAll({
      where: {
        [Op.or]: [{ email: uId }, { badge: uId }]
      }
    }).then(function (user) {
      return user[0];
    }).catch(err => {
      return undefined;
    });;
  },

  getLoggedIn(uId){
    return user.findAll({
      where: {
        [Op.and]: [{loggedIn: true}, {[Op.or]: [{email: uId}, {badge: uId}]}]
      },
      raw: true
    }).then(results => {
      if(results.length > 0)
        return results[0];
      else
        return false;
    }).catch(err => {
      return false;
    });
  },

  //Usage: Remove a user from the DB
  //Arguments:
  //         bId - User's badge id.
  //Exceptions: ConnectionError, DatabaseError, QueryError,
  // ValidationError, Other
  //Description:  This function will take a user's badge id
  // and then completely remove them from the database except for
  // and logs with their badge id.
  //
  //NOTE:  This Method is still incomplete!  Only Use
  //       for testing purposes.
  deleteUser(bId) {
    return user.destroy({
      where: {
        badge: bId
      }
    }).then(function () {
      return true;
    }).catch(err => {
      return false;
    });
  },

  //Usage:  Return's a JSON Object of all stations a user has access to.
  //Arguments:
  //          bId - The badge id for the user
  //Exceptions: ConnectionError, DatabaseError, QueryError,
  // ValidationError, Other
  //Descriptions:  This function will take a user's badge # and return
  // a JSON object with all the stations the user has been trained on and
  // can use.
  getPrivileges(bId) {
    return privileges.findAll({
      where: { badge: bId },
      raw: true
    }).then(results => {
      if (results.length < 1)
        return false;
      else
        return results;
    }).catch(err => {
      return false;
    });
  },

  //Usage:  Update's a users account to grant access to a station
  //Arguments:
  //         bId -  The badge id for the user
  //         sId -  The station id
  //Exceptions: ConnectionError, DatabaseError, QueryError,
  // ValidationError, Other
  //Description:  This function will grant a user's
  // machine usage privileges.
  grantPrivileges(bId, sId) {
    var returnStatus = { result: true, detail: 'Success' };
    return privileges.create({
      badge: bId,
      sId: sId,
      createdAt: new Date(),
      updatedAt: new Date()
    }).then(() => {
      return returnStatus;
    }).catch(err => {
      return module.exports.errorHandling(err, returnStatus);
    });
  },

  //Usage:  Update's a users account to remove access to a station
  //Arguments:
  //         bId -  The badge id for the user
  //         sId -  The station id
  //Exceptions: ConnectionError, DatabaseError, QueryError,
  // ValidationError, Other
  //Description:  This function will update a user's
  // station privileges to 'untrained'
  removePrivileges(bId, sId) {
    var returnStatus = { result: true, detail: 'Success' };
    return privileges.destroy({
      where: {
        [Op.and]: [{ badge: bId }, { sId: sId }]
      }
    }).then(results => {
      if (results === 0) {
        returnStatus.result = false;
        returnStatus.detail = 'No matching rows';
      }
      return returnStatus;
    }).catch(err => {
      return module.exports.errorHandling(err, returnStatus);
    });
  },

  //Usage: Returns a list of registered or unregistered stations
  //Arguments:
  //         regTypeRequest - true value indicates to return registered stations
  //                        - false value indicates to return unregistered stations
  //                        - undefined (default/emtpy) value indicates to return all stations
  //Exceptions: ConnectionError, DatabaseError, QueryError,
  // ValidationError, Other
  //Description: This function returns a JSON object of all stations that match regTypeRequest.
  //Returns: registered stations if regTypeRequest is true
  //         unregistered stations if regTypeRequest is flase
  //         false if database error
  //         undefined if invalid argument was passed
  getStations(regTypeRequest) {
    if (regTypeRequest === undefined) {
      //return all stations if registered type not specified;
      return machine.findAll({
        raw: true
      }).then(stations => {
        return stations;
      }).catch(err => {
        return false;
      });
    } else if (typeof regTypeRequest !== 'boolean') {
      //Usage error: type of parameter neither boolean nor undefined
      return undefined;  //signal Error to client
    } else {
      //set flag to denote whether to search for true (registered) or flase (unregistered) stations
      var registeredFlag = (regTypeRequest === true) ? true : false;

      //return only stations that have not been registerred
      return machine.findAll({
        where: { registered: registeredFlag },
        raw: true
      }).then(stations => {
        return stations;
      }).catch(err => {
        return false;
      });
    }
  },

  // Given a station ID it will return all users who are privileged to use that station
  getPrivilegedStationUsers(sId) {
    return user.findAll({
      include: [{
        model: privileges,
        where: { sId: sId }
      }]
    }).catch(err => {
      return err;
    });
  },

  getStation(sId) {
    var returnStatus = { result: true, detail: 'Success' };  //Status Code
    return machine.findAll({
      where: {
        sId: sId
      }
    }).then(result => {
      return result[0];
    }).catch(err => {
      return module.exports.errorHandling(err, returnStatus);
    });
  },

  //Usage:  Add's a new station to the DB
  //Arguments:
  //         stationData - A JSON object with all station data
  //Exceptions: ConnectionError, DatabaseError, QueryError,
  // ValidationError, Other
  //Description:  This function will take a station data JSON object
  // and add the new station to the database.
  createStation(stationData) {
    var returnStatus = { result: true, detail: 'Success' };  //Status Code

    return machine.create({
      sId: stationData.sId,
      name: stationData.name,
      description: stationData.description,
      registered: stationData.registered,
      certCN: stationData.certCN,
      createdAt: new Date(),
      updatedAt: new Date()
    }).then(function (result) {
      return returnStatus;
    }).catch(err => {
      return module.exports.errorHandling(err, returnStatus);
    });
  },

  //Usage:  Modifies the data of a station in the database
  //Arguments:
  //         sId - Station id used to identify which station to modify
  //         stationData -  New data to be added/updated
  //Exceptions: ConnectionError, DatabaseError, QueryError,
  // ValidationError, Other
  //Description:  This function will modify all values passed in the stationData
  // argument for the matching station id.
  modifyStation(sId, stationData) {
    var returnStatus = { result: true, detail: 'Success' };        //Status Code

    return machine.update(stationData, { where: { sId: sId } }).then(results => {
      if (results[0] === 1) {
        return returnStatus;
      } else {
        returnStatus.result = false;
        returnStatus.detail = 'Invalid Data';
        return returnStatus;
      }
    }).catch(err => {
      return module.exports.errorHandling(err, returnStatus);
    });
  },

  //Usage:  Removes a stations from the database
  //Arguments:
  //          sId - Station id used to identify which station to remove
  //Exceptions: ConnectionError, DatabaseError, QueryError,
  // ValidationError, Other
  //Description:  This function will remove any instance of the station in the
  // DB based off the passed in station id argument.
  deleteStation(sId) {
    var returnStatus = { result: true, detail: 'Success' };

    return machine.destroy({
      where: {
        sId: sId
      }
    }).then(function (result) {
      if (result === 0) {
        returnStatus.result = false;
        returnStatus.detail = 'Failure to delete';
      }
      return returnStatus;
    }).catch(err => {
      returnStatus.result = false;
      returnStatus.detail = err;
      return returnStatus;
    });
  },

  //Usage:  Log's all database related events.
  //Arguments:
  //         eventClass - Type of event (e.g. badge-in, internal server error)
  //         event      - Stringed description of the event
  //         date       - Date in which the event occurred.
  //         entity     - An arbitrary identifier string for the entity responsible
  //                        for the log entry (user's badge, etc.)
  //Exceptions: ConnectionError, DatabaseError, QueryError,
  // ValidationError, Other
  //Description:  This function will record all interactivity between
  // the users and the database.  As well as all errors that occurred.
  logEvent(eventClass, entity, eventText, eventDate = undefined) {
    var returnStatus = { result: true, detail: 'Success' };
    let logData = {
      eventClass: eventClass, // A string representing the sort of event (e.g. access, error, management, start-stop)
      event: eventText, // The explanatory string describing the event (i.e. the text of the log entry)
    }
    if (eventDate) {
      logData['eventDate'] = eventDate
    }
    if (entity) {
      logData["entity"] = entity;
    }

    //Adds a new row to the event table
    return log.create(logData).then(function (result) {
      //Transaction completed
      return returnStatus;
    }).catch(err => {
      //Transaction incomplete -
      //Error occurred when adding data to the
      //Event table.
      console.error(err);
      return module.exports.errorHandling(err, returnStatus);
    });
  },

  //Usage:  Returns All, or some, events.
  //Arguments:
  //         eventClass - Type of event (e.g. badge-in, internal server error)
  //         from       - Starting date
  //         to         - Ending date
  //         entity     - An arbitrary identifier string for the entity responsible
  //                        for the log entry (user's badge, etc.)
  //Exceptions: ConnectionError, DatabaseError, QueryError,
  // ValidationError, Other
  //Description:  This function will return a JSON object with all the events
  // that match the eventClass argument.
  // Additionally, this function will allow for requesting events
  // during a specific date range.  If both from and to are left undefined then all
  // results are returned.
  getEvents(eventClass = undefined, entity = undefined, from = undefined, to = undefined){
    var queryParameters = {};
    var returnStatus = { result: true, detail: 'Success' };
    if (typeof (eventClass) != 'undefined') {
      queryParameters["eventClass"] = eventClass;
    }
    if (typeof (from) != 'undefined' && typeof (to) != 'undefined') {
      queryParameters["eventDate"] = {
        [Op.gte]: from,
        [Op.lte]: to
      }
    } else if (typeof (from) != 'undefined') {
      queryParameters["eventDate"] = {
        [Op.gte]: from
      }
    } else if (typeof (to) != 'undefined') {
      queryParameters["eventDate"] = {
        [Op.lte]: to
      }
    }
    if (typeof(entity) != 'undefined') {
      queryParameters["entity"] = entity;
    }
    return log.findAll({
      where: queryParameters,
      order: [['eventDate', 'DESC']]
    }).then(function (data) {
      return data;
    }).catch(err => {
      console.error(err);
      return [];
    });
  },

  //Usage:  Returns All, or some, events.
  //Arguments:
  //         Id - Event identifier integer, as returned by getEvents
  //Exceptions: ConnectionError, DatabaseError, QueryError,
  // ValidationError, Other
  //Description:  This function will delete an event with the specified
  // ID, if it exists. It returns a status object.
  deleteEvent(eventId) {
    return log.destroy({
      where: {
        id: eventId
      }
    }).then(function () {
      return { result: true, detail: 'Success' };
    }).catch(err => {
      return { result: false, detail: err };
    });
  }
}
