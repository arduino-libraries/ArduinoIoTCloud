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

#include "ConnectionManager.h"

class GSMConnectionManager : public ConnectionManager {
public:
  GSMConnectionManager(const char *pin, const char *apn, const char *login, const char *pass);

  virtual unsigned long getTime();
  virtual void init();
  virtual void check();
  virtual Client &getClient() { return networkClient; };
  virtual UDP &getUDP() { return udp; };

  GSMClient networkClient;
  GSM gsmAccess;
  GPRS gprs;
  GSMUDP udp;

private:

  void changeConnectionState(NetworkConnectionState _newState);

  const int CHECK_INTERVAL_IDLE = 100;
  const int CHECK_INTERVAL_INIT = 100;
  const int CHECK_INTERVAL_CONNECTING = 500;
  const int CHECK_INTERVAL_GETTIME = 666;
  const int CHECK_INTERVAL_CONNECTED = 10000;
  const int CHECK_INTERVAL_RETRYING = 5000;
  const int CHECK_INTERVAL_DISCONNECTED = 1000;
  const int CHECK_INTERVAL_ERROR = 500;

  const int MAX_GETTIME_RETRIES = 30;

  const char *pin, *apn, *login, *pass;
  unsigned long lastConnectionTickTime;
  unsigned long getTimeRetries;
  int connectionTickTimeInterval;

};

static const unsigned long NETWORK_CONNECTION_INTERVAL = 30000;

GSMConnectionManager::GSMConnectionManager(const char *pin, const char *apn, const char *login, const char *pass) :
  pin(pin),
  apn(apn),
  login(login),
  pass(pass),
  lastConnectionTickTime(millis()),
  connectionTickTimeInterval(CHECK_INTERVAL_IDLE),
  getTimeRetries(MAX_GETTIME_RETRIES) {
}

unsigned long GSMConnectionManager::getTime() {
  return gsmAccess.getTime();
}

void GSMConnectionManager::init() {
  char msgBuffer[120];
  if (gsmAccess.begin(pin) == GSM_READY) {
    debugMessage("SIM card ok", 2);
    gsmAccess.setTimeout(CHECK_INTERVAL_RETRYING);
    changeConnectionState(CONNECTION_STATE_CONNECTING);
  } else {
    debugMessage("SIM not present or wrong PIN", 0);
    while(1);
  }
}

void GSMConnectionManager::changeConnectionState(NetworkConnectionState _newState) {
  char msgBuffer[120];
  int newInterval = CHECK_INTERVAL_IDLE;
  switch (_newState) {
    case CONNECTION_STATE_INIT:
      newInterval = CHECK_INTERVAL_INIT;
      break;
    case CONNECTION_STATE_CONNECTING:
      sprintf(msgBuffer, "Connecting to Cellular Network");
      debugMessage(msgBuffer, 2);
      newInterval = CHECK_INTERVAL_CONNECTING;
      break;
    case CONNECTION_STATE_GETTIME:
      debugMessage("Acquiring Time from Network", 3);
      newInterval = CHECK_INTERVAL_GETTIME;
      getTimeRetries = MAX_GETTIME_RETRIES;
      break;
    case CONNECTION_STATE_CONNECTED:
      newInterval = CHECK_INTERVAL_CONNECTED;
      break;
    case CONNECTION_STATE_DISCONNECTED:
      if(netConnectionState == CONNECTION_STATE_CONNECTED){
        debugMessage("Disconnected from Cellular Network", 0);
        debugMessage("Attempting reconnection", 0);
      }else if(netConnectionState == CONNECTION_STATE_GETTIME){
        debugMessage("Connection to Cellular Network lost during Time acquisition.\nAttempting reconnection", 0);
      }
      newInterval = CHECK_INTERVAL_DISCONNECTED;
      break;
    case CONNECTION_STATE_ERROR:
      debugMessage("GPRS attach failed\nMake sure the antenna is connected and reset your board.", 0);
      break;
  }
  connectionTickTimeInterval = newInterval;
  lastConnectionTickTime = millis();
  netConnectionState = _newState;
}

void GSMConnectionManager::check() {
  char msgBuffer[120];
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
        sprintf(msgBuffer, "GPRS.attachGPRS(): %d", networkStatus);
        debugMessage(msgBuffer, 3);
        if (networkStatus == GSM3_NetworkStatus_t::ERROR) {
          // NO FURTHER ACTION WILL FOLLOW THIS
          changeConnectionState(CONNECTION_STATE_ERROR);
          return;
        }
        debugMessage("Sending PING to outer space...", 2);
        int pingResult;
        pingResult = gprs.ping("google.com");
        sprintf(msgBuffer, "GSM.ping(): %d", pingResult);
        debugMessage(msgBuffer, 2);
        if (pingResult < 0) {
          debugMessage("PING failed", 0);
          sprintf(msgBuffer, "Retrying in  \"%d\" milliseconds", connectionTickTimeInterval);
          debugMessage(msgBuffer, 2);
          return;
        } else {
          sprintf(msgBuffer, "Connected to GPRS Network");
          debugMessage(msgBuffer, 2);
          changeConnectionState(CONNECTION_STATE_GETTIME);
          return;
        }
        break;
      case CONNECTION_STATE_GETTIME:
        debugMessage("Acquiring Time from Network", 3);
        unsigned long networkTime;
        networkTime = getTime();
        debugMessage(".", 3, false, false);
        if(networkTime > lastValidTimestamp){
          lastValidTimestamp = networkTime;
          sprintf(msgBuffer, "Network Time: %u", networkTime);
          debugMessage(msgBuffer, 3);
          changeConnectionState(CONNECTION_STATE_CONNECTED);
        }else if(gsmAccess.isAccessAlive() != 1){
          changeConnectionState(CONNECTION_STATE_DISCONNECTED);
        }else if (!getTimeRetries--) {
           changeConnectionState(CONNECTION_STATE_DISCONNECTED);
        }
        break;
      case CONNECTION_STATE_CONNECTED:
        gsmAccessAlive = gsmAccess.isAccessAlive();
        sprintf(msgBuffer, "GPRS.isAccessAlive(): %d", gsmAccessAlive);
        debugMessage(msgBuffer, 4);
        if (gsmAccessAlive != 1) {
          changeConnectionState(CONNECTION_STATE_DISCONNECTED);
          return;
        }
        sprintf(msgBuffer, "Connected to Cellular Network");
        debugMessage(msgBuffer, 4);
        break;
      case CONNECTION_STATE_DISCONNECTED:
        gprs.detachGPRS();
        changeConnectionState(CONNECTION_STATE_CONNECTING);
        break;
    }
    lastConnectionTickTime = now;
  }
}
