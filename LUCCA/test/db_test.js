/******************************************************************
  * Copyright (c) 2018 Daniel Eynis, Bishoy Hanna, Bryan Mikkelson,
  * Justin Moore, Huy Nguyen, Michael Olivas, Andrew Wood          
  * This program is licensed under the "MIT License".              
  * Please see the file LICENSE in the source                      
  * distribution of this software for license terms.               
/******************************************************************/

var chai = require('chai');
var request = require('request');
var assert = chai.assert;
var expect = chai.expect;
var controllers = require('../database/controllers/dbm');
var db = require('../database/models');

var emptyData = {};

var incompleteData = {
  "badge": null,
  "first": "Rory",
  "last": "Eats",
  "email": "123@gmail.com",
  "phone": "(555)555-5555",
  "signature": "Rory Eats",
  "ecName": "Hello HowareU",
  "ecRel": "Father",
  "ecPhone": "(555)444-4444"
};

var trueData = {
  "badge": '9090909090',
  "first": "Rory",
  "last": "Eats",
  "email": "123@gmail.com",
  "phone": "(555)555-5555",
  "signature": "Rory Eats",
  "ecName": "Hello HowareU",
  "ecRel": "Father",
  "ecPhone": "(555)444-4444"
};

var trueData2 = {
  "badge": '8674848400101',
  "first": "Rory",
  "last": "Eats",
  "email": "123@gmail.com",
  "phone": "(555)555-5555",
  "signature": "Rory Eats",
  "ecName": "Hello HowareU",
  "ecRel": "Father",
  "ecPhone": "(555)444-4444"
};

var stationData = {
  sId: '123ABC',
  name: 'My nam Jef',
  description: 'Jef the man',
  registered: false,
  certCN: 'localhost'
}

describe('DB TEST', function () {

  describe('Connect to DB', function () {
    it('Should connect to the database', function (done) {
      db.sequelize
        .authenticate()
        .then(() => {
          done();
        })
        .catch(err => {
          done(err);
        });
    });
  });

  /*
      -------------Station Tests-------------
  */

  describe('createStation', function () {
    it('Create a station with given info', function (done) {
      controllers.createStation(stationData).then(function (results) {
        expect(results.result).to.equal(true);
        controllers.getStation(stationData.sId).then(station => {
          expect(station.sId).to.equal(stationData.sId);
          expect(station.name).to.equal(stationData.name);
          expect(station.description).to.equal(stationData.description);
          expect(station.registered).to.equal(stationData.registered);
          done();
        });
      });
    });

    it('Should reject if a station with that ID already exits', function (done) {
      controllers.createStation(stationData).then(function (results) {
        expect(results.result).to.equal(false);
        done();
      });
    });
  });

  describe('modifyStation', function () {
    it('Modify a station with some info', function (done) {
      stationData.description = 'NEW DESC';
      stationData.registered = true;
      controllers.modifyStation(stationData.sId, stationData).then(function (results) {
        expect(results.result).to.equal(true);
        controllers.getStation(stationData.sId).then(function (station) {
          expect(station.sId).to.equal(stationData.sId);
          expect(station.name).to.equal(stationData.name);
          expect(station.description).to.equal(stationData.description);
          expect(station.registered).to.equal(stationData.registered);
          done();
        });
      });
    });

    it('Should reject if no station found', function (done) {
      controllers.modifyStation('No existent ID', stationData).then(function (results) {
        expect(results.result).to.equal(false);
        done();
      });
    });
  });

  describe('deleteStation', function () {
    it('Should delete the station with given ID', function (done) {
      controllers.deleteStation(stationData.sId).then(function (result) {
        expect(result.result).to.equal(true);
        controllers.getStation(stationData.sId).then(function (result) {
          expect(result).to.equal(undefined);
          done();
        });
      });
    });

    it('Should return false if no station found', function (done) {
      controllers.deleteStation('No existent ID').then(function (results) {
        expect(results.result).to.equal(false);
        done();
      });
    });
  });

  /*
      -------------createUser Tests-------------
  */

  describe('createUser Empty Dataset', function () {
    it('Empty JSON object Should Cause Fail', function (done) {
      controllers.createUser(emptyData).then(function (results) {
        expect(results.result).to.equal(false);
        done();
      });
    });
  });

  describe('createUser Incomplete Dataset', function () {
    it('Null Data Entry Should Cause Fail', function (done) {
      controllers.createUser(incompleteData).then(function (results) {
        expect(results.result).to.equal(false);
        done();
      });
    });
  });

  describe('createUser Complete Dataset', function () {
    it('Complete Data should create new User', function (done) {
      controllers.createUser(trueData).then(function (results) {
        expect(results.result).to.equal(true);
        done();
      });
    });
  });

  describe('validateUser with badge', function () {
    it('Return the userData object as specified by createUser input', function (done) {
      controllers.validateUser(trueData['badge']).then(function (results) {
        expect(results['badge']).to.equal(trueData['badge']);
        done();
      });
    });
  });

  describe('validateUser with email', function () {
    it('Return the userData object as specified by createUser input', function (done) {
      controllers.validateUser(trueData['email']).then(function (results) {
        expect(results['email']).to.equal(trueData['email']);
        done();
      });
    });
  });

  describe('validateUser no user exists', function () {
    it('Return undefined', function (done) {
      controllers.validateUser('U DONT EXIST').then(function (results) {
        expect(results).to.equal(undefined);
        done();
      });
    });
  });

  describe('createUser primary Key violation', function () {
    it('Same complete data should cause primary key fail', function (done) {
      controllers.createUser(trueData).then(function (results) {
        expect(results.result).to.equal(false);
        done();
      });
    });
  });

  describe('createUser unique email constraint', function () {
    it('Should cause unique key fail', function (done) {
      controllers.createUser(trueData2).then(function (results) {
        expect(results.result).to.equal(false);
        done();
      });
    });
  });

  describe('deleteUser', function () {
    it('Remove New User by id', function (done) {
      controllers.deleteUser(trueData.badge).then(function (results) {
        expect(results).to.equal(true);
        done();
      });
    });
  });

  //Should return full list of stations - both registered and unregistered
  describe('GetStations: ALL', function () {
    it('Return all stations (currently 6)', function (done) {
      controllers.getStations().then(function (results) {
        expect(results).to.have.length.above(3);
        done();
      });
    });
  });

  //Should return list of registered stations only
  describe('GetStations: true', function () {
    it('Return REGISTERED stations (currently 3)', function (done) {
      controllers.getStations(true).then(function (results) {
        expect(results).to.have.length.above(1);
        done();
      });
    });
  });

  //Should return list of registered stations only
  describe('GetStations: specific (registered)', function () {
    it('Verify a station known to be registered is returned ("WOOOOOO")', function (done) {
      controllers.getStations(true).then(function (results) {
        var containsWOOOOOO = false;
        //look for the name of a station that is known to be registered 
        results.forEach(element => {
          if (element.name === "WOOOOOO") {
            containsWOOOOOO = true;
          };
        });
        expect(containsWOOOOOO).to.equal(true);
        done();
      });
    });
  });

  //Should return only stations that are NOT registered
  describe('GetStations: false', function () {
    it('Return UNREGISTERED stations (currently 3)', function (done) {
      controllers.getStations(false).then(function (results) {
        expect(results).to.have.length.above(1);
        done();
      });
    });
  });

  //Should return list of registered stations only
  describe('GetStations: specific (unregistered)', function () {
    it('Verify a station known to be UNREGISTERED is returned ("Hacksaw")', function (done) {
      controllers.getStations(false).then(function (results) {
        var containsHacksaw = false;
        //look for the name of a station that is known to be registered 
        results.forEach(element => {
          if (element.name === "Hacksaw") {
            containsHacksaw = true;
          };
        });
        expect(containsHacksaw).to.equal(true);
        done();
      });
    });
  });

  /*
      -------------getPrivileges Tests-------------
  */

  describe('getPrivileges invalid badge id', function () {
    it('Should return false', function (done) {
      controllers.getPrivileges('fake badge id').then(results => {
        expect(results).to.equal(false);
        done();
      }).catch(err => {
        console.log(err);
        done();
      });
    });
  });

  describe('getPrivileges valid badge id', function () {
    it('Should return results', function (done) {
      controllers.getPrivileges('1234567').then(results => {
        expect(results).to.have.length.above(0);
        done();
      }).catch(err => {
        console.log(err);
        done();
      });
    });
  });

/*
    -------------logEvent Tests-------------
*/

  describe('logEvent create generic event', function(){
    it('Should create a new event and return true', function(done){
      controllers.logEvent('test_event', 'test_event_1', "This is a simple event that occurred").done(function(results){
        expect(results.result).to.equal(true);
        done();
      });
    });
  });

  describe('logEvent create generic event with timestamp for now', function(){
    it('Should create a new event and return true', function(done){
     controllers.logEvent('test_event', 'test_event_2', "This is a simple event with an explicit timestamp that's now", new Date()).done(function(results){
       expect(results.result).to.equal(true);
       done();
     });
    });
  });

  describe('logEvent create generic event with old timestamp', function(){
    it('Should create a new event and return true', function(done){
      controllers.logEvent('test_event', 'test_event_3', "This is a simple event with an explicit timestamp from long ago", new Date('12-13-1998')).done(function(results){
        expect(results.result).to.equal(true);
        done();
      });
    });
  });

  describe('logEvent create an event with a different event class', function(){
    it('Should create a new event and return true', function(done){
      controllers.logEvent('also_test_event', 'test_event_4', "This is a simple event that occurred and is filed under a different event class").done(function(results){
        expect(results.result).to.equal(true);
        done();
      });
    });
  });

/*
    -------------getEvents Tests-------------
*/

  describe('getEvents testing', function(){
    it('Get all logged events (should return a list containing at least four events)', function(done){
      controllers.getEvents().then(function(results){
        expect(results).to.be.an('array').that.has.lengthOf.above(3);
        done();
      });
    });

    it('Get all logged events with class test_event (should return a list containing at least three events)', function(done){
      controllers.getEvents('test_event').then(function(results){
        expect(results).to.be.an('array').that.has.lengthOf.above(2);
        for (var i = 0; i < results.length; i++){
          expect(results[i].eventClass).to.be.a('string').that.equals('test_event');
        }
        done();
      });
    });
    it('Get all events logged with an event_class of also_test_event (should return a list containing one event)', function(done){
      controllers.getEvents('also_test_event').then(function(results){
        expect(results).to.be.an('array').that.has.lengthOf.above(0);
        for (var i = 0; i < results.length; i++){
          expect(results[i].eventClass).to.be.a('string').that.equals('also_test_event');
        }
        done();
      });
    });
    it('Get all events logged with an event_class of no_such_class (should return a list containing nothing)', function(done){
      controllers.getEvents('no_such_class').then(function(results){
        expect(results).to.be.an('array').that.has.lengthOf(0);
        done();
      });
    });
    it('Get all events logged after 2005 (should return a list containing one event)', function(done){
      controllers.getEvents(undefined, undefined, from=new Date('1-1-2005')).then(function(results){
        expect(results).to.be.an('array').that.has.lengthOf.above(2);
        for (var i = 0; i < results.length; i++){
          expect(results[i].eventDate).to.be.a('date').that.is.above(from);
        }
        done();
      });
    });
    it('Get all events logged before 2005 (should return a list containing three events)', function(done){
      controllers.getEvents(undefined, undefined,  undefined, to=new Date('1-1-2005')).then(function(results){
        expect(results).to.be.an('array').that.has.lengthOf.above(0);
        for (var i = 0; i < results.length; i++){
          expect(results[i].eventDate).to.be.a('date').that.is.not.above(to);
        }
        done();
      });
    });
    it('Get all events logged after 2050 (should return a list containing zero events)', function(done){
      controllers.getEvents(undefined, undefined,  from=new Date('1-1-2050')).then(function(results){
        expect(results).to.be.an('array').that.has.lengthOf(0);
        done();
      });
    });
    it('Get all events logged after 2005 but before 2050 (should return a list containing three events)', function(done){
      controllers.getEvents(undefined, undefined,  from=new Date('1-1-2005'), to=new Date('1-1-2050')).then(function(results){
        expect(results).to.be.an('array').that.has.lengthOf.above(2);
        for (var i = 0; i < results.length; i++){
          expect(results[i].eventDate).to.be.a('date').that.is.above(from).and.not.above(to);
        }
        done();
      });
    });
    it('Get all events logged after 2005 but before 2050 with event class test_event (should return a list containing two events)', function(done){
      controllers.getEvents('test_event', undefined,  from=new Date('1-1-2005'), to=new Date('1-1-2050')).then(function(results){
        expect(results).to.be.an('array').that.has.lengthOf.above(1);
        for (var i = 0; i < results.length; i++){
          expect(results[i].eventClass).to.be.a('string').that.equals('test_event');
          expect(results[i].eventDate).to.be.a('date').that.is.above(from).and.not.above(to);
        }
        done();
      });
    });
  });

/*
    -------------getEvents Tests-------------
*/

  describe('deleteEvent testing', function(){
    it('Delete an event', function(done){
      controllers.getEvents().then(function(results){
        var first = results[0].id;
        controllers.deleteEvent(first).then(function(results_2){
          controllers.getEvents().then(function(remaining){
            for (var i = 0; i < remaining.length; i++){
              expect(remaining[i].id).to.not.equal(first);
            }
            done();
          });
        }).catch(err => {
          console.log(err);
          done();
        });
      });
    });
  });

/*
    -------------modifyUser Tests-------------
*/

  describe('getPrivileges empty string id', function () {
    it('Should return false', function (done) {
      controllers.getPrivileges('').then(results => {
        expect(results).to.equal(false);
        done();
      }).catch(err => {
        console.log(err);
        done();
      });
    });
  });

  describe('getPrivileges null string', function () {
    it('Should return false', function (done) {
      controllers.getPrivileges(null).then(results => {
        expect(results).to.equal(false);
        done();
      }).catch(err => {
        console.log(err);
        done();
      });
    });
  });

  describe('getPrivileges undefined string', function () {
    it('Should return false', function (done) {
      controllers.getPrivileges().then(results => {
        expect(results).to.equal(false);
        done();
      }).catch(err => {
        console.log(err);
        done();
      });
    });
  });

  /*
      -------------getUsers Tests-------------
  */

  var startDate;
  var endDate;

  describe('getUser + two undefined dates', function () {
    it('Should return full set of users', function (done) {
      controllers.getUsers(startDate, endDate).then(function (results) {
        //console.log(results);
        expect(results).to.have.length.above(1);
        //expect(results.result).to.equal(true);
        done();
        startDate = '2000-01-01';
      });
    });
  });

  describe('getUser + one undefined date', function () {
    it('Should return subset of users from a start date', function (done) {
      controllers.getUsers(startDate, endDate).then(function (results) {
        expect(results).to.have.length.above(1);
        done();
        endDate = '2000-01-02'
      });
    });
  });

  describe('getUser + two dates + out of range', function () {
    it('Should return empty set of data', function (done) {
      controllers.getUsers(startDate, endDate).then(function (results) {
        expect(results).to.have.length(0);
        done();
        startDate = '2018-02-02';
        endDate = '3000-02-04';
      });
    });
  });

  describe('getUser + two dates + in range', function () {
    it('Should return subset of data', function (done) {
      controllers.getUsers(startDate, endDate).then(function (results) {
        expect(results).to.have.length.above(0);
        done();
      });
    });
  });

  /*
      -------------modifyUser Tests-------------
  */

  describe('modifyUser invalid badge id', function () {
    it('Should not update and return status false', function (done) {
      controllers.modifyUser('fake badge id', { 'password': 'fake' }).then(function (results) {
        expect(results.result).to.equal(false);
        done();
      });
    });
  });

  describe('modifyUser + valid badge id + valid attribute field + null value', function () {
    it('Should not update table.  Desired update attribute cannot be null.  Return status false', function (done) {
      controllers.modifyUser('1234567', { 'first': null }).then(function (results) {
        expect(results.result).to.equal(false);
        done();
      });
    });
  });

  describe('modifyUser + valid badge id + valid attribute field + null value', function () {
    it('Should update table.  Desired attribute can be null.  Return status true', function (done) {
      controllers.modifyUser('1234567', { 'password': null }).then(function (results) {
        expect(results.result).to.equal(true);
        done();
      });
    });
  });

  describe('modifyUser + valid badge id + valid attribute field + value', function () {
    it('Should update table attribute, return status true', function (done) {
      controllers.modifyUser('1234567', { 'password': 'fart' }).then(function (results) {
        expect(results.result).to.equal(true);
        done();
      });
    });
  });

  describe('modifyUser + valid badge id + invalid attribute field', function () {
    it('Should not update table attribute.  Return status true', function (done) {
      controllers.modifyUser('1234567', { 'passowrd': 'lalala' }).then(function (results) {
        expect(results.result).to.equal(true);
        done();
      });
    });
  });

  /*
      -------------grantPrivileges Tests-------------
  */
  describe('grantPrivileges Tests', function (){
    it('Grant new privileges + null badge id, will fail and return false', function(done){
      controllers.grantPrivileges(null, '3').then(results => {
        expect(results.result).to.equal(false);
        done();
      });
    });

    it('Grant new privileges + null station id, will fail and return false', function(done){
      controllers.grantPrivileges('1234567', null).then(results => {
        expect(results.result).to.equal(false);
        done();
      });
    });

    it('Grant new privileges + undefined arguments will fail and return false', function(done){
      controllers.grantPrivileges().then(results => {
        expect(results.result).to.equal(false);
        done();
      });
    });

    it('Grant new privileges + badge ID not in user table will fail and return false', function(done){
      controllers.grantPrivileges('This is a test', '3').then(results => {
        expect(results.result).to.equal(false);
        done();
      });
    });

    it('Grant new privileges + station ID not in machines table will fail and return false', function(done){
      controllers.grantPrivileges('1234567', '400').then(results => {
        expect(results.result).to.equal(false);
        done();
      });
    });

    it('Grant new privileges + badge/station ID in users/machines table will succeed and return true', function(done){
      controllers.grantPrivileges('1234567', '3').then(results => {
        expect(results.result).to.equal(true);
        done();
      });
    });
  });

  /*
     -----------removePrivileges Tests-----------------
   */
  describe('removePrivileges Tests', function () {
    it('Should grant privilege to a user.  Return status true', function (done) {
      controllers.grantPrivileges('1234567', '3').then(results => {
        expect(results.result).to.equal(true);
        done();
      });
    });

    it('Should return false with empty badge id + valid station', function (done) {
      controllers.removePrivileges('', '3').then(results => {
        expect(results.result).to.equal(false);
        done();
      });
    });

    it('Should return false with null badge id + valid station', function (done) {
      controllers.removePrivileges(null, '3').then(results => {
        expect(results.result).to.equal(false);
        done();
      });
    });

    it('Should return false with invalid badge id + valid station', function (done) {
      controllers.removePrivileges('fake bade id yo', '3').then(results => {
        expect(results.result).to.equal(false);
        done();
      });
    });

    it('Should return false with valid badge id + empty station', function (done) {
      controllers.removePrivileges('1234567', '').then(results => {
        expect(results.result).to.equal(false);
        done();
      });
    });

    it('Should return false with valid badge id + null station', function (done) {
      controllers.removePrivileges('1234567', null).then(results => {
        expect(results.result).to.equal(false);
        done();
      });
    });

    it('Should return false with valid badge id + invalid station', function (done) {
      controllers.removePrivileges('1234567', 'fake station id yo').then(results => {
        expect(results.result).to.equal(false);
        done();
      });
    });

    it('Should return false with undefined arguments', function (done) {
      controllers.removePrivileges().then(results => {
        expect(results.result).to.equal(false);
        done();
      });
    });

    it('Should return true valid badge + valid station', function (done) {
      controllers.removePrivileges('1234567', '3').then(results => {
        expect(results.result).to.equal(true);
        done();
      });
    });
  });

  describe('getLoggedIn tests', function(){
    it('User 1234567 should be logged in', function(done){
      controllers.modifyUser('1234567', {'loggedIn': true}).then(t => {
        controllers.getLoggedIn('1234567').then(results => {
          expect(results.loggedIn).to.equal(true);
          done();
        });
      });
    });

    it('User 1234567 should be logged out', function(done){
      controllers.modifyUser('1234567', {'loggedIn': false}).then(t => {
        controllers.getLoggedIn('1234567').then(results => {
          expect(results).to.equal(false);
          done();
        });
      });
    });

    it('User abc@123.com should be logged in', function(done){
      controllers.modifyUser('1234567', {'loggedIn': true}).then(t => {
        controllers.getLoggedIn('abc@123.com').then(results => {
          expect(results.loggedIn).to.equal(true);
          done();
        });
      });
    });

    it('User abc@123.com should be logged out', function(done){
      controllers.modifyUser('1234567', {'loggedIn': false}).then(t => {
        controllers.getLoggedIn('abc@123.com').then(results => {
          expect(results).to.equal(false);
          done();
        });
      });
    });
  });
});
