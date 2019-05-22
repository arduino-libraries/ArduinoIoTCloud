/*
   This file is part of ArduinoIoTCloud.

   Copyright 2019 ARDUINO SA (http://www.arduino.cc/)

   This software is released under the GNU General Public License version 3,
   which covers the main part of arduino-cli.
   The terms of this license can be found at:
   https://www.gnu.org/licenses/gpl-3.0.en.html

   You can be released from the requirements of the above licenses by purchasing
   a commercial license. Buying such a license is mandatory if you want to modify or
   otherwise use the software for commercial activities involving the Arduino
   software without disclosing the source code of your own applications. To purchase
   a commercial license, send an email to license@arduino.cc.
*/

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include "GSMConnectionManager.h"

#ifdef BOARD_HAS_GSM /* Only compile if this is a board with GSM */

/******************************************************************************
   CONSTANTS
 ******************************************************************************/

static const unsigned long NETWORK_CONNECTION_INTERVAL = 30000;

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

GSMConnectionManager::GSMConnectionManager(const char *pin, const char *apn, const char *login, const char *pass) :
  pin(pin),
  apn(apn),
  login(login),
  pass(pass),
  lastConnectionTickTime(millis()),
  connectionTickTimeInterval(CHECK_INTERVAL_IDLE) {
}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

void GSMConnectionManager::init() {
  if (gsmAccess.begin(pin) == GSM_READY) {
    debugMessage(DebugLevel::Info, "SIM card ok");
    gsmAccess.setTimeout(CHECK_INTERVAL_RETRYING);
    changeConnectionState(NetworkConnectionState::CONNECTING);
  } else {
    debugMessage(DebugLevel::Error, "SIM not present or wrong PIN");
    while (1);
  }
}

unsigned long GSMConnectionManager::getTime() {
  return gsmAccess.getTime();
}

void GSMConnectionManager::check() {
  unsigned long const now = millis();
  int gsmAccessAlive;
  if (now - lastConnectionTickTime > connectionTickTimeInterval) {
    switch (netConnectionState) {
      case NetworkConnectionState::INIT: {
          init();
        }
        break;
      case NetworkConnectionState::CONNECTING: {
          // NOTE: Blocking Call when 4th parameter == true
          GSM3_NetworkStatus_t networkStatus;
          networkStatus = gprs.attachGPRS(apn, login, pass, true);
          debugMessage(DebugLevel::Debug, "GPRS.attachGPRS(): %d", networkStatus);
          if (networkStatus == GSM3_NetworkStatus_t::ERROR) {
            // NO FURTHER ACTION WILL FOLLOW THIS
            changeConnectionState(NetworkConnectionState::ERROR);
            return;
          }
          debugMessage(DebugLevel::Info, "Sending PING to outer space...");
          int pingResult;
          pingResult = gprs.ping("time.arduino.cc");
          debugMessage(DebugLevel::Info, "GSM.ping(): %d", pingResult);
          if (pingResult < 0) {
            debugMessage(DebugLevel::Error, "PING failed");
            debugMessage(DebugLevel::Info, "Retrying in  \"%d\" milliseconds", connectionTickTimeInterval);
            return;
          } else {
            debugMessage(DebugLevel::Info, "Connected to GPRS Network");
            changeConnectionState(NetworkConnectionState::CONNECTED);
            return;
          }
        }
        break;
      case NetworkConnectionState::CONNECTED: {
          gsmAccessAlive = gsmAccess.isAccessAlive();
          debugMessage(DebugLevel::Verbose, "GPRS.isAccessAlive(): %d", gsmAccessAlive);
          if (gsmAccessAlive != 1) {
            changeConnectionState(NetworkConnectionState::DISCONNECTED);
            return;
          }
          debugMessage(DebugLevel::Verbose, "Connected to Cellular Network");
        }
        break;
      case NetworkConnectionState::GETTIME: {
          /* Do nothing */
        }
        break;
      case NetworkConnectionState::DISCONNECTING: {
          /* Do nothing */
        }
        break;
      case NetworkConnectionState::DISCONNECTED: {
          gprs.detachGPRS();
          changeConnectionState(NetworkConnectionState::CONNECTING);
        }
        break;
      case NetworkConnectionState::ERROR: {
          /* Do nothing */
        }
        break;
    }
    lastConnectionTickTime = now;
  }
}

/******************************************************************************
   PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

void GSMConnectionManager::changeConnectionState(NetworkConnectionState const newState) {
  netConnectionState = newState;

  switch (netConnectionState) {
    case NetworkConnectionState::INIT: {
        connectionTickTimeInterval = CHECK_INTERVAL_INIT;
      }
      break;
    case NetworkConnectionState::CONNECTING: {
        debugMessage(DebugLevel::Info, "Connecting to Cellular Network");
        connectionTickTimeInterval = CHECK_INTERVAL_CONNECTING;
      }
      break;
    case NetworkConnectionState::CONNECTED: {
        connectionTickTimeInterval = CHECK_INTERVAL_CONNECTED;
      }
      break;
    case NetworkConnectionState::DISCONNECTED: {
        if (netConnectionState == NetworkConnectionState::CONNECTED) {
          debugMessage(DebugLevel::Error, "Disconnected from Cellular Network");
          debugMessage(DebugLevel::Error, "Attempting reconnection");
        }
        connectionTickTimeInterval = CHECK_INTERVAL_DISCONNECTED;
      }
      break;
    case NetworkConnectionState::ERROR: {
        debugMessage(DebugLevel::Error, "GPRS attach failed\n\rMake sure the antenna is connected and reset your board.");
      }
      break;
    default: {
        connectionTickTimeInterval = CHECK_INTERVAL_IDLE;
      }
      break;
  }
  lastConnectionTickTime = millis();
}

#endif /* #ifdef BOARD_HAS_GSM  */
