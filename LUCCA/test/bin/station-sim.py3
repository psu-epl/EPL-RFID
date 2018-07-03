#!/usr/bin/python3

#################################################################
# Copyright (c) 2018 Daniel Eynis, Bishoy Hanna, Bryan Mikkelson,
# Justin Moore, Huy Nguyen, Michael Olivas, Andrew Wood          
# This program is licensed under the "MIT License".              
# Please see the file LICENSE in the source                      
# distribution of this software for license terms.               
#################################################################

"""
This script provides a way to generate simulated API actions similar to that expected from the LUCCA station hardware. It also validates any responses sent by the server software to whatever degree is possible.

Usage text can be obtained by invoking this script from a command line with the '-h' or '--help' flag.

Alternately, it can be imported into another Python project and used as a library.
"""

import requests, argparse, re, sys

#import urllib3
#urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)
# uncomment this line locally if you want to disable securiyt warnings from urllib3. however, this line causes errors with travis CI.

_defaults = {
  'controller' : None,
  'station_id' : '1',
  'user_id'    : u'KREUGERF1001',
  'timeout'    : float(2),
}

def validate_response(response, request, assertions):
  def validate_header_assertion(header, pattern, response):
    try:
      if not re.match(pattern, response.headers[header]):
        raise ValueError("pattern constraint '{0}' on header '{1}' does not match received value '{2}'".format(pattern, header, response.headers[header]))
    except KeyError:
      raise KeyError("expected header '{0}' not present".format(header))
    except TypeError as e:
      if 'expected string or bytes-like object' in e.args:
        raise ValueError("value of header '{0}' was null or otherwise unparseable".format(header))
      else:
        raise

  def validate_body_assertion(pattern, response):
    try:
      body = response.text[:200]
      if not re.match(pattern, body):
        raise ValueError("pattern constraint '{0}' on response body does not match received value '{1}'{2}".format(pattern, body, " (truncated)" if len(body) < len(response.text) else ""))
    except TypeError as e:
      if 'expected string or bytes-like object' in e.args:
        raise ValueError("value of body was null or otherwise unparseable".format(header))
      else:
        raise

  def validate_rcode_assertion(codes, response):
    if not response.status_code in codes:
      raise ValueError("list of expected status codes ({0}) does not contain the response code received ({1})".format(', '.join([str(x) for x in codes]), response.status_code))

  failures = 0

  for header, pattern in assertions['headers'].items():
    try:
      validate_header_assertion(header, pattern, response)
    except (KeyError, ValueError) as e:
      failures += 1
      print("Response validation failure in headers: " + str(e))

  try:
    validate_body_assertion(assertions['body'], response)
  except ValueError as e:
    failures += 1
    print("Response validation failure in body: " + str(e))

  try:
    validate_rcode_assertion(assertions['rcode'], response)
  except ValueError as e:
    failures += 1
    print("Response validation failure: " + str(e))

  for conditions, cond_assertions in assertions['conditional']:
    # Test to see if the conditions hold true
    try:
      # Check all header conditions
      for header, pattern in conditions['headers'].items():
        validate_header_assertion(header, pattern, response)
      # Check body condition
      validate_body_assertion(assertions['body'], response)
      # Check response code condition
      validate_rcode_assertion(assertions['rcode'], response)
    except (KeyError, ValueError):
      # If any condition does not hold, the conditional assertion is vacuous - stop processing and move on
      continue

    try:
      for header, pattern in cond_assertions['headers'].items():
        try:
          validate_header_assertion(header, pattern, response)
        except (KeyError, ValueError) as e:
          failures += 1
          print("Response validation failure in headers : " + str(e))
    except KeyError:
        # A KeyError caught here indicates there are no conditional assertions on the response
        pass

    try:
      validate_body_assertion(cond_assertions['body'], response)
    except ValueError as e:
      failures += 1
      print("Response validation failure in body: " + str(e))
    except KeyError:
      # this indicates no conditional assertions on the body
      pass

    try:
      validate_rcode_assertion(cond_assertions['rcode'], response)
    except ValueError as e:
      failures += 1
      print("Response validation failure: " + str(e))
    except KeyError:
      # this indicates no conditional assertions on the rsponse code
      pass

  if failures:
    print("{0} failures were encountered while validating the endpoint response".format(failures))
    return False
  else:
    print("Response valid, all constraints met (status code {0})".format(response.status_code))
    return True

def user_access(cert, use_ssl=True, verify=True, controller=_defaults['controller'], station_id=_defaults['station_id'], user_id=_defaults['user_id'], active=False, **kwargs):
  """
  Send a user-access API action, as detailed in section 4.2. of the station API specification
  """
  schema = 'https://' if use_ssl else 'http://'
  headers={ 'Station-ID' : station_id,
            'Station-State' : 'Enabled' if active else 'Disabled',
            'Content-Type' : 'text/plain' }
  s = requests.Session()
  req = requests.Request('POST', schema + controller + '/api/user-access', data=user_id, headers=headers)
  prep = req.prepare()
  try:
    resp = s.send(prep, cert=cert, verify=verify)
  except requests.exceptions.ConnectionError as e:
    print("Response error: " + str(e))
    return False

  assertions = {
             'headers'     : {}, # No universal header assertions
             'body'        : r'^(OK|Unauthorized|Forbidden)$',
             'rcode'       : [200, 401, 403], # These are the only expected response codes (400 indicates an error in this test)
             'conditional' : [ # Conditional tests to run based on circumstances go here
                               ( # First conditional assertion 
                                 # Response should include a valid Station-State header if rcode is "200 OK"
                                 { # List of conditions that must be true for the assertions to be tested
                                   'rcode' : [200] # Header test 1 ("if this")
                                   # Additional header tests would go here ("...and this")
                                 },
                                 { # List of assertions to be tested if the above conditions hold true
                                   'headers' : {
                                                 'Station-State' : r'^(Enabled|Disabled)$' # Assertion 1 ("then this")
                                                 # Additional conditions for response headers would go here ("...and this")
                                               }
                                   # Additional conditions for other scopes besides header could be listed here
                                 }
                               ),
                               ( # Second conditional assertion
                                 # If response code is "200 OK" and station is enabled, expect valid User-ID-String header
                                 {
                                   'headers' : {
                                                 'Station-State' : r'^Enabled$'
                                               },
                                   'rcode'   : [200]
                                 },
                                 {
                                   'headers' : {
                                                 'User-ID-String' : r'^.{1,60}$'
                                               }
                                 }
                               )
                                 # Additional conditional tests would go here
                             ]
           }
  return validate_response(resp, prep, assertions)

def local_reset(cert, use_ssl=True, verify=True, controller=_defaults['controller'], station_id=_defaults['station_id'], **kwargs):
  """
  Send a local-reset API action, as detailed in section 4.3. of the station API specification
  """
  schema = 'https://' if use_ssl else 'http://'
  headers={ 'Station-ID' : station_id }
  s = requests.Session()
  req = requests.Request('POST', schema + controller + '/api/local-reset', headers=headers)
  prep = req.prepare()
  try:
    resp = s.send(prep, cert=cert, verify=verify)
  except requests.exceptions.ConnectionError as e:
    print("Response error: " + str(e))
    return False

  assertions = {
             'headers'     : {},
             'body'        : r'^(OK|Unauthorized|Forbidden)$',
             'rcode'       : [200, 401, 403],
             'conditional' : []
           }
  return validate_response(resp, prep, assertions)

def last_state(cert, use_ssl=True, verify=True, controller=_defaults['controller'], station_id=_defaults['station_id'], **kwargs):
  """
  Send a last-state API action, as detailed in section 4.4. of the station API specification
  """
  schema = 'https://' if use_ssl else 'http://'
  headers={ 'Station-ID' : station_id }
  s = requests.Session()
  req = requests.Request('GET', schema + controller + '/api/last-state', headers=headers)
  prep = req.prepare()
  try:
    resp = s.send(prep, cert=cert, verify=verify)
  except requests.exceptions.ConnectionError as e:
    print("Response error: " + str(e))
    return False

  assertions = {
             'headers'     : {},
             'body'        : r'^(OK|Unauthorized|Forbidden)$',
             'rcode'       : [200, 401, 403],
             'conditional' : [
                               ( # If response code is 200, Station-State header should be included
                                 {
                                   'rcode' : [200]
                                 },
                                 {
                                   'headers' : {
                                                 'Station-State' : r'^(Enabled|Disabled)$'
                                               }
                                 }
                               ),
                               ( # If response code is 200 *and* Station-State header is "Enabled", also expect an ID string
                                 {
                                   'headers' : {
                                                 'Station-State' : r'^Enabled$'
                                               },
                                   'rcode'   : [200]
                                 },
                                 {
                                   'headers' : {
                                                 'User-ID-String' : r'^.{1,60}$'
                                               }
                                 }
                               )
                             ]
           }
  return validate_response(resp, prep, assertions)

def station_heartbeat(cert, use_ssl=True, verify=True, controller=_defaults['controller'], station_id=_defaults['station_id'], user_id=_defaults['user_id'], **kwargs):
  """
  Send a station-heartbeat API action, as detailed in section 4.5. of the station API specification
  """
  schema = 'https://' if use_ssl else 'http://'
  headers = { 'Station-ID' : station_id }
  s = requests.Session()
  req = requests.Request('POST', schema + controller + '/api/station-heartbeat', data=user_id, headers=headers)
  prep = req.prepare()
  try:
    resp = s.send(prep, cert=cert, verify=verify)
  except requests.exceptions.ConnectionError as e:
    print("Response error: " + str(e))
    return False

  assertions = {
                 'headers'     : {},
                 'body'        : r'^(OK|Unauthorized|Forbidden)$',
                 'rcode'       : [200, 401, 403],
                 'conditional' : [
                                   ( # If response code is 200, expect a Date header
                                     {
                                       'rcode'   : [200]
                                     },
                                     {
                                       'headers' : {
                                                     # FIXME:Make this regex match an actual IMF-Fixdate datestamp if possible
                                                     'Date' : r'^.{30}.*$'
                                                   }
                                     }
                                   )
                                 ]
               }
  return validate_response(resp, prep, assertions)

# Processing and calls for handling invocation as a CLI script (arg parsing, etc.)
if __name__ == '__main__':
  do_action = {
    'user-access'       : user_access,
    'local-reset'       : local_reset,
    'last-state'        : last_state,
    'station-heartbeat' : station_heartbeat
  }
  parser = argparse.ArgumentParser(description="Simulate some expected station behavior based on the API standard document")
  parser.add_argument('-c', '--controller', dest='controller', default=_defaults['controller'], help="Controller URL or IP address")
  parser.add_argument('-i', '--station-id', dest='station_id', default=_defaults['station_id'], help="Station-ID header value to pass in the chosen action")
  parser.add_argument('-u', '--user-id', dest='user_id', default=_defaults['user_id'], help="User ID token (not display name), passed as the request body for user-access actions. Ignored when specified with other action types.")
  parser.add_argument('-t', '--timeout', dest='timeout', default=_defaults['timeout'], help="Request timeout duration for HTTP requests used to invoke API actions. [default: 2.0 sec]")
  parser.add_argument('--no-verify', dest='verify', const=False, action='store_const', default=True)
  parser.add_argument('--station-active', dest='active', const=True, action='store_const', default=False)
  cert_parser = parser.add_mutually_exclusive_group(required=True)
  cert_parser.add_argument('-C', '--certificate', dest='cert', help="Path to a PEM certificate file")
  cert_parser.add_argument('--no-certificate', dest='cert', const=None, action='store_const')
  cert_parser.add_argument('--no-ssl', dest='use_ssl', const=False, action='store_const', default=True)
  action_parser = parser.add_mutually_exclusive_group(required=True)
  action_parser.add_argument('-U', '--user-access', dest='action', const='user-access', action='store_const')
  action_parser.add_argument('-R', '--local-reset', dest='action', const='local-reset', action='store_const')
  action_parser.add_argument('-L', '--last-state', dest='action', const='last-state', action='store_const')
  action_parser.add_argument('-H', '--station-heartbeat', dest='action', const='station-heartbeat', action='store_const')
  action_parser.add_argument('-A', '--action-type', dest='action', choices=['user-access', 'local-reset', 'last-state', 'station-heartbeat'])
  args = parser.parse_args()

  if do_action[args.action](**vars(args)):
      sys.exit(0)
  else:
      sys.exit(1)
