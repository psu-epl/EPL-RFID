/******************************************************************
  * Copyright (c) 2018 Daniel Eynis, Bishoy Hanna, Bryan Mikkelson,
  * Justin Moore, Huy Nguyen, Michael Olivas, Andrew Wood          
  * This program is licensed under the "MIT License".              
  * Please see the file LICENSE in the source                      
  * distribution of this software for license terms.               
/******************************************************************/

var chai = require('chai');
var request = require('request');
var fs = require('fs');
var assert = chai.assert;
var expect = chai.expect;
var options = {
  baseUrl: 'https://localhost:3000',
  agentOptions: {
    ca: fs.readFileSync('test/cert/certificate.pem')
  }
};


describe('Page status', function () {
  describe('Main page', function() {
    it('status', function(done) {
      request.get('/',
        options,
        function(error, response, body) {
          if (error) { console.error(error); }
          expect(response.statusCode).to.equal(200);
          done();
        }
      );
    });
  });


  describe('Admin login page', function() {
    it('status', function(done) {
      request('/adminLogin',
        options,
        function(error, response, body) {
          if (error) { console.error(error); }
          expect(response.statusCode).to.equal(200);
          done();
        }
      );
    });
  });


  describe('Badge-in page', function() {
    it('status', function(done) {
      request('/badgein',
        options,
        function(error, response, body) {
          if (error) { console.error(error); }
          expect(response.statusCode).to.equal(200);
          done();
        }
      );
    });
  });


  describe('User registration page', function() {
    it('status', function(done) {
      request('/registration/00001', 
        options,
        function(error, response, body) {
          expect(response.statusCode).to.equal(200);
         done();
        }
      );
    });
  });
  

  describe('User Management page', function() {
    it('status', function(done) {
      request('/userManagement',
        options,
        function(error, response, body) {
        if (error) { console.error(error); }
        expect(response.statusCode).to.equal(200);
        done();
      });
    });

    it('status', function(done) {
      request('/userManagement/00001',
        options,
        function(error, response, body) {
        if (error) { console.error(error); }
        expect(response.statusCode).to.equal(200);
        done();
      });
    });
  });


  describe('Admin create new account', function() {
    it('status', function(done) {
      request('/adminRegister',
        options,
        function(error, response, body) {
        if (error) { console.error(error); }
        expect(response.statusCode).to.equal(200);
        done();
      });
    });
  });


  describe('Admin reset account', function() {
    it('status', function(done) {
      request('/adminReset',
        options,
        function(error, response, body) {
        if (error) { console.error(error); }
        expect(response.statusCode).to.equal(200);
        done();
      });
    });
  });


  describe('Station management page', function() {
    it('status of all results', function(done) {
      request('/stationManagement/all',
        options,
        function(error, response, body) {
        if (error) { console.error(error); }
        expect(response.statusCode).to.equal(200);
        done();
      });
    });

    it('status of registered stations results', function(done) {
      request('/stationManagement/registered',
        options,
        function(error, response, body) {
        if (error) { console.error(error); }
        expect(response .statusCode).to.equal(200);
        done();
      });
    });

    it('status of unregistered stations results', function(done) {
      request('/stationManagement/unregistered',
        options,
        function(error, response, body) {
        if (error) { console.error(error); }
        expect(response .statusCode).to.equal(200);
        done();
      });
    });
  });


  describe('Events log', function() {
    it('should return 200 status for displaying all events', function(done) {
      request('/eventsLog/all',
        options,
        function(error, response, body) {
        if (error) { console.error(error); }
        expect(response.statusCode).to.equal(200);
        done();
      });
    });

    it('should return 200 status for displaying EPL Badge IN events', function(done) {
      request('/eventsLog/badgeIn',
        options,
        function(error, response, body) {
        if (error) { console.error(error); }
        expect(response .statusCode).to.equal(200);
        done();
      });
    });

    it('should return 200 status for displaying generic_event events', function(done) {
      request('/eventsLog/generic',
        options,
        function(error, response, body) {
        if (error) { console.error(error); }
        expect(response .statusCode).to.equal(200);
        done();
      });
    });

    it('should return 200 status for displaying different_event events', function(done) {
      request('/eventsLog/different',
        options,
        function(error, response, body) {
        if (error) { console.error(error); }
        expect(response .statusCode).to.equal(200);
        done();
      });
    });

    it('should return 200 status for displaying station-status request events', function(done) {
      request('/eventsLog/statusReq',
        options,
        function(error, response, body) {
        if (error) { console.error(error); }
        expect(response .statusCode).to.equal(200);
        done();
      });
    });

    it('should return 200 status for displaying station-reset events', function(done) {
      request('/eventsLog/stnReset',
        options,
        function(error, response, body) {
        if (error) { console.error(error); }
        expect(response .statusCode).to.equal(200);
        done();
      });
    });

    it('should return 200 status for displaying station management events', function(done) {
      request('/eventsLog/stnMngmnt',
        options,
        function(error, response, body) {
        if (error) { console.error(error); }
        expect(response .statusCode).to.equal(200);
        done();
      });
    });
  });
  
});
