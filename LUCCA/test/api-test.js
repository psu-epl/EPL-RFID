/******************************************************************
  * Copyright (c) 2018 Daniel Eynis, Bishoy Hanna, Bryan Mikkelson,
  * Justin Moore, Huy Nguyen, Michael Olivas, Andrew Wood          
  * This program is licensed under the "MIT License".              
  * Please see the file LICENSE in the source                      
  * distribution of this software for license terms.               
/******************************************************************/

var chai = require('chai');
var request = require('request');
var childProcess = require('child_process');
var assert = chai.assert;
var expect = chai.expect;
var fs = require('fs');
var controllers = require('../database/controllers/dbm');
var db = require('../database/models');
var cert = fs.readFileSync('test/cert/certificate.pem');
var admin = {
  "badge": '90909098',
  "first": "Admin",
  "last": "Last",
  "email": "Admin@gmail.com",
  "phone": "(555)555-5555",
  "signature": "Rory Eats",
  "ecName": "Hello HowareU",
  "ecRel": "Father",
  "ecPhone": "(555)444-4444"
};
var manager = {
  "badge": '86748484',
  "first": "Manager",
  "last": "Last",
  "email": "Manager@gmail.com",
  "phone": "(555)555-5555",
  "signature": "Rory Eats",
  "ecName": "Hello HowareU",
  "ecRel": "Father",
  "ecPhone": "(555)444-4444"
};
var user = {
  "badge": '79230034',
  "first": "user",
  "last": "Last",
  "email": "User@gmail.com",
  "phone": "(555)555-5555",
  "signature": "Rory Eats",
  "ecName": "Hello HowareU",
  "ecRel": "Father",
  "ecPhone": "(555)444-4444"
};
var testStationOne = {
  sId: '123ABC',
  name: 'Cutter',
  description: 'Cut shit',
  registered: true,
  certCN: 'localhost'
};
var testStationTwo = {
  sId: '456DEF',
  name: 'Grinder',
  description: 'Grind shit',
  registered: true,
  certCN: 'localhost'
};
var testStationThree = {
  sId: '789HIJ',
  name: 'Watcher',
  description: 'Watch shit',
  registered: true,
  certCN: 'localhost'
};
var testStationFour = {
  sId: '123KLM',
  name: 'Cooker',
  description: 'Cook shit',
  registered: true,
  certCN: 'localhost'
};


// create testing environment
describe('Create testing environment', function () {
  it('Create a statition one for testing', function (done) {
    controllers.createStation(testStationOne).then(function (results) {
      expect(results.result).to.equal(true);
      controllers.getStation(testStationOne.sId).then(station => {
        expect(station.sId).to.equal(testStationOne.sId);
        expect(station.name).to.equal(testStationOne.name);
        expect(station.description).to.equal(testStationOne.description);
        expect(station.registered).to.equal(testStationOne.registered);
        done();
      });
    });
  });
  it('Create station two for testing', function (done) {
    controllers.createStation(testStationTwo).then(function (results) {
      expect(results.result).to.equal(true);
      controllers.getStation(testStationTwo.sId).then(station => {
        expect(station.sId).to.equal(testStationTwo.sId);
        expect(station.name).to.equal(testStationTwo.name);
        expect(station.description).to.equal(testStationTwo.description);
        expect(station.registered).to.equal(testStationTwo.registered);
        done();
      });
    }); 
  });
  it('Create station two for testing', function (done) {
    controllers.createStation(testStationThree).then(function (results) {
      expect(results.result).to.equal(true);
      controllers.getStation(testStationThree.sId).then(station => {
        expect(station.sId).to.equal(testStationThree.sId);
        expect(station.name).to.equal(testStationThree.name);
        expect(station.description).to.equal(testStationThree.description);
        expect(station.registered).to.equal(testStationThree.registered);
        done();
      });
    }); 
  });
  it('Create station two for testing', function (done) {
    controllers.createStation(testStationFour).then(function (results) {
      expect(results.result).to.equal(true);
      controllers.getStation(testStationFour.sId).then(station => {
        expect(station.sId).to.equal(testStationFour.sId);
        expect(station.name).to.equal(testStationFour.name);
        expect(station.description).to.equal(testStationFour.description);
        expect(station.registered).to.equal(testStationFour.registered);
        done();
      });
    }); 
  });
  it('Complete Data should create new User', function (done) {
    controllers.createUser(admin).then(function (res) {
      controllers.modifyUser(admin.badge, {'loggedIn': true}).then(function (results) {
        expect(results.result).to.equal(true);
        done();
      });
    });
  });
  it('Complete Data should create new User', function (done) {
    controllers.createUser(manager).then(function (res) {
      controllers.modifyUser(manager.badge, {'loggedIn': true}).then(function (results) {
        expect(results.result).to.equal(true);
        done();
      });
    });
  });
  it('Complete Data should create new User', function (done) {
    controllers.createUser(user).then(function (res) {
      controllers.modifyUser(user.badge, {'loggedIn': true}).then(function (results) {
        expect(results.result).to.equal(true);
        done();
      });
    });
  });
  it('Should update admin to admin status', function (done) {
    controllers.modifyUser(admin.badge, { 'status': 'Admin' }).then(function (results) {
      expect(results.result).to.equal(true);
      done();
    });
  });
  it('Should update manager to manager status', function (done) {
    controllers.modifyUser(manager.badge, { 'status': 'Manager' }).then(function (results) {
      expect(results.result).to.equal(true);
      done();
    });
  });
  it('Grant privileges to admin for station 1', function(done){
    controllers.grantPrivileges(admin.badge, testStationOne.sId).then(results => {
      expect(results.result).to.equal(true);
      done();
    });
  });
  it('Grant privileges to admin for station 2', function(done){
    controllers.grantPrivileges(admin.badge, testStationTwo.sId).then(results => {
      expect(results.result).to.equal(true);
      done();
    });
  });
  it('Grant privileges to admin for station 3', function(done){
    controllers.grantPrivileges(admin.badge, testStationThree.sId).then(results => {
      expect(results.result).to.equal(true);
      done();
    });
  });
  it('Grant privileges to admin for station 4', function(done){
    controllers.grantPrivileges(admin.badge, testStationFour.sId).then(results => {
      expect(results.result).to.equal(true);
      done();
    });
  });
  it('Grant privileges to Manager for station 1', function(done){
    controllers.grantPrivileges(manager.badge, testStationOne.sId).then(results => {
      expect(results.result).to.equal(true);
      done();
    });
  });
  it('Grant privileges to Manager for station 2', function(done){
    controllers.grantPrivileges(manager.badge, testStationTwo.sId).then(results => {
      expect(results.result).to.equal(true);
      done();
    });
  });
  it('Grant privileges to Manager for station 3', function(done){
    controllers.grantPrivileges(manager.badge, testStationThree.sId).then(results => {
      expect(results.result).to.equal(true);
      done();
    });
  });
  it('Grant privileges to user for station 3', function(done){
    controllers.grantPrivileges(user.badge, testStationThree.sId).then(results => {
      expect(results.result).to.equal(true);
      done();
    });
  });
});


describe('Test API user-access logging in and logging off', function () {
  it('should add an admin to machine', function(done) {
    let cmd = 'python3 test/bin/station-sim.py3 -C test/cert/combined.pem -c localhost:3001 --no-verify -A user-access -i ' + testStationOne.sId + ' -u ' + admin.badge;
    childProcess.exec(cmd, function(error, stdout, stderr){
      expect(stdout).to.equal('Response valid, all constraints met (status code 200)\n');
      done();
    });
  });
  it('should add manager to machine', function(done) {
    let cmd = 'test/bin/station-sim.py3 -C test/cert/combined.pem -c localhost:3001 --no-verify -A user-access -i ' + testStationTwo.sId + ' -u ' + manager.badge;
    childProcess.exec(cmd, function(error, stdout, stderr){
      expect(stdout).to.equal('Response valid, all constraints met (status code 200)\n');
      done();
    });
  });
  it('should add user to machine', function(done) {
    let cmd = 'test/bin/station-sim.py3 -C test/cert/combined.pem -c localhost:3001 --no-verify -A user-access -i ' + testStationThree.sId + ' -u ' + user.badge;
    childProcess.exec(cmd, function(error, stdout, stderr){
      expect(stdout).to.equal('Response valid, all constraints met (status code 200)\n');
      done();
    });
  });
  it('should log admin off machine', function(done) {
    let cmd = 'test/bin/station-sim.py3 -C test/cert/combined.pem -c localhost:3001 --no-verify --station-active -A user-access -i ' + testStationOne.sId + ' -u ' + admin.badge;
    childProcess.exec(cmd, function(error, stdout, stderr){
      expect(stdout).to.equal('Response valid, all constraints met (status code 200)\n');
      done();
    });
  });
  it('should log manager off machine', function(done) {
    let cmd = 'test/bin/station-sim.py3 -C test/cert/combined.pem -c localhost:3001 --no-verify --station-active -A user-access -i ' + testStationTwo.sId + ' -u ' + manager.badge;
    childProcess.exec(cmd, function(error, stdout, stderr){
      expect(stdout).to.equal('Response valid, all constraints met (status code 200)\n');
      done();
    });
  });
  it('should log user off machine', function(done) {
    let cmd = 'test/bin/station-sim.py3 -C test/cert/combined.pem -c localhost:3001 --no-verify --station-active -A user-access -i ' + testStationThree.sId + ' -u ' + user.badge;
    childProcess.exec(cmd, function(error, stdout, stderr){
      expect(stdout).to.equal('Response valid, all constraints met (status code 200)\n');
      done();
    });
  });
});


describe('Reset API user-access, users can not kick off other users', function () { 
  it('Should log manager onto machine 3', function(done) {
    let cmd = 'test/bin/station-sim.py3 -C test/cert/combined.pem -c localhost:3001 --no-verify -A user-access -i ' + testStationThree.sId + ' -u ' + manager.badge;
    childProcess.exec(cmd, function(error, stdout, stderr){
      expect(stdout).to.equal('Response valid, all constraints met (status code 200)\n');
      done();
    });
  });
  it('Should keep the user from logging onto machine 3, because a manager is on it status 403', function(done) {
    let cmd = 'test/bin/station-sim.py3 -C test/cert/combined.pem -c localhost:3001 --no-verify --station-active -A user-access -i ' + testStationThree.sId + ' -u ' + user.badge;
    childProcess.exec(cmd, function(error, stdout, stderr){
      expect(stdout).to.equal('Response valid, all constraints met (status code 403)\n');
      done();
    });
  });
  it('should log manager off machine 3', function(done) {
    let cmd = 'test/bin/station-sim.py3 -C test/cert/combined.pem -c localhost:3001 --no-verify --station-active -A user-access -i ' + testStationThree.sId + ' -u ' + manager.badge;
    childProcess.exec(cmd, function(error, stdout, stderr){
      expect(stdout).to.equal('Response valid, all constraints met (status code 200)\n');
      done();
    });
  });
  it('should log an admin onto machine 2', function(done) {
    let cmd = "test/bin/station-sim.py3 -C test/cert/combined.pem -c localhost:3001 --no-verify -A user-access -i " + testStationTwo.sId + " -u " + admin.badge + "\n";
    childProcess.exec(cmd, function(error, stdout, stderr){
      expect(stdout).to.equal('Response valid, all constraints met (status code 200)\n');
      done();
    });
  });
  it('Should keep manager from logging onto machine 2 status because a admin is on it, return status 403', function(done) {
    let cmd = 'test/bin/station-sim.py3 -C test/cert/combined.pem -c localhost:3001 --no-verify --station-active -A user-access -i ' + testStationTwo.sId + ' -u ' + manager.badge;
    childProcess.exec(cmd, function(error, stdout, stderr){
      expect(stdout).to.equal('Response valid, all constraints met (status code 403)\n');
      done();
    });
  });
  it('should log admin off machine', function(done) {
    let cmd = 'test/bin/station-sim.py3 -C test/cert/combined.pem -c localhost:3001 --no-verify --station-active -A user-access -i ' + testStationTwo.sId + ' -u ' + admin.badge;
    childProcess.exec(cmd, function(error, stdout, stderr){
      expect(stdout).to.equal('Response valid, all constraints met (status code 200)\n');
      done();
    });
  });
});


describe('Teset API user-access, test caching.', function () { 
  it('should add user to cache so they can be granted permission to station 1', function(done) {
    let cmd = 'test/bin/station-sim.py3 -C test/cert/combined.pem -c localhost:3001 --no-verify -A user-access -i ' + testStationOne.sId + ' -u ' + user.badge;
    childProcess.exec(cmd, function(error, stdout, stderr){
      expect(stdout).to.equal('Response valid, all constraints met (status code 403)\n');
      done();
    });
  });
  it('Admin will give the user in cache privileges to use machine 1', function(done) {
    let cmd = 'test/bin/station-sim.py3 -C test/cert/combined.pem -c localhost:3001 --no-verify -A user-access -i ' + testStationOne.sId + ' -u ' + admin.badge;
    childProcess.exec(cmd, function(error, stdout, stderr){
      expect(stdout).to.equal('Response valid, all constraints met (status code 200)\n');
      done();
    });
  });
  it('should log user off machine', function(done) {
    let cmd = 'test/bin/station-sim.py3 -C test/cert/combined.pem -c localhost:3001 --no-verify --station-active -A user-access -i ' + testStationOne.sId + ' -u ' + user.badge;
    childProcess.exec(cmd, function(error, stdout, stderr){
      expect(stdout).to.equal('Response valid, all constraints met (status code 200)\n');
      done();
    });
  });
  it('should add user to cache so they can be granted permission to station 2', function(done) {
    let cmd = 'test/bin/station-sim.py3 -C test/cert/combined.pem -c localhost:3001 --no-verify -A user-access -i ' + testStationTwo.sId + ' -u ' + user.badge;
    childProcess.exec(cmd, function(error, stdout, stderr){
      expect(stdout).to.equal('Response valid, all constraints met (status code 403)\n');
      done();
    });
  });
  it('should allow manager to grant access to user to station 2', function(done) {
    let cmd = 'test/bin/station-sim.py3 -C test/cert/combined.pem -c localhost:3001 --no-verify -A user-access -i ' + testStationTwo.sId + ' -u ' + manager.badge;
    childProcess.exec(cmd, function(error, stdout, stderr){
      expect(stdout).to.equal('Response valid, all constraints met (status code 200)\n');
      done();
    });
  });
  it('should log user off machine', function(done) {
    let cmd = 'test/bin/station-sim.py3 -C test/cert/combined.pem -c localhost:3001 --no-verify --station-active -A user-access -i ' + testStationTwo.sId + ' -u ' + user.badge;
    childProcess.exec(cmd, function(error, stdout, stderr){
      expect(stdout).to.equal('Response valid, all constraints met (status code 200)\n');
      done();
    });
  });
  it('should add user to cache so they can be granted permission to station 4', function(done) {
    let cmd = 'test/bin/station-sim.py3 -C test/cert/combined.pem -c localhost:3001 --no-verify -A user-access -i ' + testStationFour.sId + ' -u ' + user.badge;
    childProcess.exec(cmd, function(error, stdout, stderr){
      expect(stdout).to.equal('Response valid, all constraints met (status code 403)\n');
      done();
    });
  });
  it('should not allow manager to grant privs to user because manager does not have prives to station 4,', function(done) {
    let cmd = 'test/bin/station-sim.py3 -C test/cert/combined.pem -c localhost:3001 --no-verify -A user-access -i ' + testStationFour.sId + ' -u ' + manager.badge;
    childProcess.exec(cmd, function(error, stdout, stderr){
      expect(stdout).to.equal('Response valid, all constraints met (status code 403)\n');
      done();
    });
  });
});


describe('Test API user-access, not allow a user to log in more than 1 machine', function () {
  it('should add an admin to machine', function(done) {
    let cmd = 'test/bin/station-sim.py3 -C test/cert/combined.pem -c localhost:3001 --no-verify -A user-access -i ' + testStationOne.sId + ' -u ' + admin.badge;
    childProcess.exec(cmd, function(error, stdout, stderr){
      expect(stdout).to.equal('Response valid, all constraints met (status code 200)\n');
      done();
    });
  });
  it('should return 403, not allow admin to be on 2 machines', function(done) {
    let cmd = 'test/bin/station-sim.py3 -C test/cert/combined.pem -c localhost:3001 --no-verify -A user-access -i ' + testStationTwo.sId + ' -u ' + admin.badge;
    childProcess.exec(cmd, function(error, stdout, stderr){
      expect(stdout).to.equal('Response valid, all constraints met (status code 403)\n');
      done();
    });
  });
})


describe('Clean up testing environment', function () {
  it('Should delete test station one', function (done) {
    controllers.deleteStation(testStationOne.sId).then(function (result) {
      expect(result.result).to.equal(true);
      controllers.getStation(testStationOne.sId).then(function (result) {
        expect(result).to.equal(undefined);
        done();
     });
    });
  });
  it('Should delete test station two', function (done) {
    controllers.deleteStation(testStationTwo.sId).then(function (result) {
      expect(result.result).to.equal(true);
      controllers.getStation(testStationTwo.sId).then(function (result) {
        expect(result).to.equal(undefined);
        done();
      });
    });
  });
  it('Should delete test station three', function (done) {
    controllers.deleteStation(testStationThree.sId).then(function (result) {
      expect(result.result).to.equal(true);
      controllers.getStation(testStationThree.sId).then(function (result) {
        expect(result).to.equal(undefined);
        done();
      });
    });
  });
  it('Should delete test station four', function (done) {
    controllers.deleteStation(testStationFour.sId).then(function (result) {
      expect(result.result).to.equal(true);
      controllers.getStation(testStationFour.sId).then(function (result) {
        expect(result).to.equal(undefined);
        done();
      });
    });
  });
  it('Should remove admin', function (done) {
    controllers.deleteUser(admin.badge).then(function (results) {
      expect(results).to.equal(true);
      done();
    });
  });
  it('Should remove manager', function (done) {
    controllers.deleteUser(manager.badge).then(function (results) {
      expect(results).to.equal(true);
      done();
    });
  });
  it('Should remove user', function (done) {
    controllers.deleteUser(user.badge).then(function (results) {
      expect(results).to.equal(true);
      done();
    });
  });
});
