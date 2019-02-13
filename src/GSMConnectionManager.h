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

#include <MKRGSM.h>
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
  const int CHECK_INTERVAL_GETTIME = 1000;
  const int CHECK_INTERVAL_CONNECTED = 10000;
  const int CHECK_INTERVAL_RETRYING = 5000;
  const int CHECK_INTERVAL_DISCONNECTED = 1000;
  const int CHECK_INTERVAL_ERROR = 500;

  const char *pin, *apn, *login, *pass;
  unsigned long lastConnectionTickTime, lastNetworkStep;
  int connectionTickTimeInterval;
  GSMUDP Udp;
  void sendNTPpacket(const char * address, uint8_t* packetBuffer);
  unsigned long getNTPTime();
};

static const unsigned long NETWORK_CONNECTION_INTERVAL = 30000;

GSMConnectionManager::GSMConnectionManager(const char *pin, const char *apn, const char *login, const char *pass) :
  pin(pin),
  apn(apn),
  login(login),
  pass(pass),
  lastConnectionTickTime(millis()),
  connectionTickTimeInterval(CHECK_INTERVAL_IDLE) {
}

unsigned long GSMConnectionManager::getTime() {
  return gsmAccess.getTime();
}

void GSMConnectionManager::init() {
  char msgBuffer[120];
  if (gsmAccess.begin(pin) == GSM_READY) {
    *msgBuffer = 0;
    sprintf(msgBuffer, "SIM card ok");
    debugMessage(msgBuffer, 2);
    gsmAccess.setTimeout(CHECK_INTERVAL_RETRYING);
  } else {
    *msgBuffer = 0;
    sprintf(msgBuffer, "SIM not present");
    debugMessage(msgBuffer, 2);
    while(1);
  }
}

void GSMConnectionManager::changeConnectionState(NetworkConnectionState _newState) {
  netConnectionState = _newState;
  int newInterval = CHECK_INTERVAL_IDLE;
  switch (_newState) {
    case CONNECTION_STATE_INIT:
      newInterval = CHECK_INTERVAL_INIT;
      break;
    case CONNECTION_STATE_CONNECTING:
      newInterval = CHECK_INTERVAL_CONNECTING;
      break;
    case CONNECTION_STATE_GETTIME:
      newInterval = CHECK_INTERVAL_GETTIME;
      break;
    case CONNECTION_STATE_CONNECTED:
      newInterval = CHECK_INTERVAL_CONNECTED;
      break;
    case CONNECTION_STATE_DISCONNECTED:
      newInterval = CHECK_INTERVAL_DISCONNECTED;
      break;
  }
  connectionTickTimeInterval = newInterval;
  lastConnectionTickTime = millis();
}

void GSMConnectionManager::check() {
  char msgBuffer[120];
  unsigned long now = millis();
  int networkStatus = 0;
  if (now - lastConnectionTickTime > connectionTickTimeInterval) {
    switch (netConnectionState) {
      case CONNECTION_STATE_INIT:
        init();
        changeConnectionState(CONNECTION_STATE_CONNECTING);
        break;
      case CONNECTION_STATE_CONNECTING:
        // blocking call with 4th parameter == true
        networkStatus = gprs.attachGPRS(apn, login, pass, true);
        *msgBuffer = 0;
        sprintf(msgBuffer, "GPRS.attachGPRS(): %d", networkStatus);
        debugMessage(msgBuffer, 2);
        if (networkStatus == ERROR) {
          debugMessage("GPRS attach failed\nMake sure the antenna is connected", 0);
          changeConnectionState(CONNECTION_STATE_CONNECTING);
          lastConnectionTickTime = now;
          return;
        }
        *msgBuffer = 0;
        sprintf(msgBuffer, "Trying to ping external world");
        debugMessage(msgBuffer, 2);

        networkStatus = gprs.ping("google.com");
        *msgBuffer = 0;
        sprintf(msgBuffer, "GSM.ping(): %d", networkStatus);
        debugMessage(msgBuffer, 2);
        if (networkStatus < 0) {
          *msgBuffer = 0;
          sprintf(msgBuffer, "Ping failed");
          debugMessage(msgBuffer, 0);

          *msgBuffer = 0;
          sprintf(msgBuffer, "Retrying in  \"%d\" milliseconds", connectionTickTimeInterval);
          debugMessage(msgBuffer, 2);
          changeConnectionState(CONNECTION_STATE_CONNECTING);
          return;
        } else {
          *msgBuffer = 0;
          sprintf(msgBuffer, "Connected!");
          debugMessage(msgBuffer, 2);
          changeConnectionState(CONNECTION_STATE_GETTIME);
          return;
        }
        break;
      case CONNECTION_STATE_GETTIME:
        debugMessage("Acquiring Time from Network", 3);
        unsigned long networkTime;
        networkTime = getTime();
        *msgBuffer = 0;
        sprintf(msgBuffer, "Network Time: %u", networkTime);
        debugMessage(msgBuffer, 3);
        if(networkTime > lastValidTimestamp){
          lastValidTimestamp = networkTime;
          changeConnectionState(CONNECTION_STATE_CONNECTED);
        }
        break;
      case CONNECTION_STATE_CONNECTED:
        // keep testing connection
        networkStatus = gsmAccess.isAccessAlive();
        *msgBuffer = 0;
        sprintf(msgBuffer, "GPRS.isAccessAlive(): %d", networkStatus);
        debugMessage(msgBuffer, 4);
        if (networkStatus != 1) {
          changeConnectionState(CONNECTION_STATE_DISCONNECTED);
          return;
        }
        *msgBuffer = 0;
        sprintf(msgBuffer, "Still connected");
        debugMessage(msgBuffer, 4);
        break;
      case CONNECTION_STATE_DISCONNECTED:
        gprs.detachGPRS();

        *msgBuffer = 0;
        sprintf(msgBuffer, "DISC | GPRS.status(): %d", gprs.status());
        debugMessage(msgBuffer, 1);
        *msgBuffer = 0;
        sprintf(msgBuffer, "Connection lost.");
        debugMessage(msgBuffer, 0);
        debugMessage("Attempting reconnection", 1);
        changeConnectionState(CONNECTION_STATE_CONNECTING);
        //wifiClient.stop();
        break;
    }
    lastConnectionTickTime = now;
  }
}
