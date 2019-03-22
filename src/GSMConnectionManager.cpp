/*
 * This file is part of ArduinoIoTCloud.
 *
 * Copyright 2019 ARDUINO SA (http://www.arduino.cc/)
 *
 * This software is released under the GNU General Public License version 3,
 * which covers the main part of arduino-cli.
 * The terms of this license can be found at:
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * You can be released from the requirements of the above licenses by purchasing
 * a commercial license. Buying such a license is mandatory if you want to modify or
 * otherwise use the software for commercial activities involving the Arduino
 * software without disclosing the source code of your own applications. To purchase
 * a commercial license, send an email to license@arduino.cc.
 */

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include "GSMConnectionManager.h"

#ifdef BOARD_HAS_GSM /* Only compile if this is a board with GSM */

/******************************************************************************
 * CONSTANTS
 ******************************************************************************/

static const unsigned long NETWORK_CONNECTION_INTERVAL = 30000;

/******************************************************************************
 * CTOR/DTOR
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
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

void GSMConnectionManager::init() {
  char msgBuffer[120];
  if (gsmAccess.begin(pin) == GSM_READY) {
    debugMessage(2, "SIM card ok");
    gsmAccess.setTimeout(CHECK_INTERVAL_RETRYING);
    changeConnectionState(CONNECTION_STATE_CONNECTING);
  } else {
    debugMessage(0, "SIM not present or wrong PIN");
    while(1);
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
      case CONNECTION_STATE_INIT:
        init();
        break;
      case CONNECTION_STATE_CONNECTING:
        // NOTE: Blocking Call when 4th parameter == true
        GSM3_NetworkStatus_t networkStatus;
        networkStatus = gprs.attachGPRS(apn, login, pass, true);
        debugMessage(3, "GPRS.attachGPRS(): %d", networkStatus);
        if (networkStatus == GSM3_NetworkStatus_t::ERROR) {
          // NO FURTHER ACTION WILL FOLLOW THIS
          changeConnectionState(CONNECTION_STATE_ERROR);
          return;
        }
        debugMessage(2, "Sending PING to outer space...");
        int pingResult;
        pingResult = gprs.ping("time.arduino.cc");
        debugMessage(2, "GSM.ping(): %d", pingResult);
        if (pingResult < 0) {
          debugMessage(0, "PING failed");
          debugMessage(2, "Retrying in  \"%d\" milliseconds", connectionTickTimeInterval);
          return;
        } else {
          debugMessage(2, "Connected to GPRS Network");
          changeConnectionState(CONNECTION_STATE_CONNECTED);
          return;
        }
        break;
      case CONNECTION_STATE_CONNECTED:
        gsmAccessAlive = gsmAccess.isAccessAlive();
        debugMessage(4, "GPRS.isAccessAlive(): %d", gsmAccessAlive);
        if (gsmAccessAlive != 1) {
          changeConnectionState(CONNECTION_STATE_DISCONNECTED);
          return;
        }
        debugMessage(4, "Connected to Cellular Network");
        break;
      case CONNECTION_STATE_DISCONNECTED:
        gprs.detachGPRS();
        changeConnectionState(CONNECTION_STATE_CONNECTING);
        break;
    }
    lastConnectionTickTime = now;
  }
}

/******************************************************************************
 * PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

void GSMConnectionManager::changeConnectionState(NetworkConnectionState _newState) {
  char msgBuffer[120];
  int newInterval = CHECK_INTERVAL_IDLE;
  switch (_newState) {
    case CONNECTION_STATE_INIT:
      newInterval = CHECK_INTERVAL_INIT;
      break;
    case CONNECTION_STATE_CONNECTING:
      debugMessage(2, "Connecting to Cellular Network");
      newInterval = CHECK_INTERVAL_CONNECTING;
      break;
    case CONNECTION_STATE_CONNECTED:
      newInterval = CHECK_INTERVAL_CONNECTED;
      break;
    case CONNECTION_STATE_DISCONNECTED:
      if(netConnectionState == CONNECTION_STATE_CONNECTED){
        debugMessage(0, "Disconnected from Cellular Network");
        debugMessage(0, "Attempting reconnection");
      }
      newInterval = CHECK_INTERVAL_DISCONNECTED;
      break;
    case CONNECTION_STATE_ERROR:
      debugMessage(0, "GPRS attach failed\n\rMake sure the antenna is connected and reset your board.");
      break;
  }
  connectionTickTimeInterval = newInterval;
  lastConnectionTickTime = millis();
  netConnectionState = _newState;
}

#endif /* #ifdef BOARD_HAS_GSM  */