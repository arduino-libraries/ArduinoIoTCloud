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

class WiFiConnectionManager : public ConnectionManager {
public:
  WiFiConnectionManager(const char *ssid, const char *pass);

  virtual unsigned long getTime();
  virtual void init();
  virtual void check();
  virtual Client &getClient() { return wifiClient; };
  

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

  const char *ssid, *pass;
  unsigned long lastConnectionTickTime, lastNetworkStep;
  WiFiClient wifiClient;
  int connectionTickTimeInterval;
};

static const unsigned long NETWORK_CONNECTION_INTERVAL = 30000;

WiFiConnectionManager::WiFiConnectionManager(const char *ssid, const char *pass) :
  ssid(ssid), pass(pass),
  lastConnectionTickTime(millis()),
  connectionTickTimeInterval(CHECK_INTERVAL_IDLE) {
}

unsigned long WiFiConnectionManager::getTime() {
  return WiFi.getTime();
}

void WiFiConnectionManager::init() {
}

void WiFiConnectionManager::changeConnectionState(NetworkConnectionState _newState) {
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

void WiFiConnectionManager::check() {
  char msgBuffer[120];
  unsigned long now = millis();
  int networkStatus = 0;
  if (now - lastConnectionTickTime > connectionTickTimeInterval) {
    switch (netConnectionState) {
      case CONNECTION_STATE_IDLE:
        changeConnectionState(CONNECTION_STATE_INIT);
        break;
      case CONNECTION_STATE_INIT:
        networkStatus = WiFi.status();
        *msgBuffer = 0;
        sprintf(msgBuffer, "WiFi.status(): %d", networkStatus);
        debugMessage(msgBuffer, 2);
        if (networkStatus == NETWORK_HARDWARE_ERROR) {
          debugMessage("WiFi Hardware not available\nMake sure you are using a WiFi enabled board/shield", 0);
          // don't continue:
          changeConnectionState(CONNECTION_STATE_ERROR);
          lastConnectionTickTime = now;
          return;
        }
        *msgBuffer = 0;
        sprintf(msgBuffer, "WiFi Firmware v. %s", WiFi.firmwareVersion());
        debugMessage(msgBuffer, 0);
        changeConnectionState(CONNECTION_STATE_CONNECTING);
        break;
      case CONNECTION_STATE_CONNECTING:
        *msgBuffer = 0;
        sprintf(msgBuffer, "Connecting to \"%s\"", ssid);
        debugMessage(msgBuffer, 2);

        networkStatus = WiFi.begin(ssid, pass);
        *msgBuffer = 0;
        sprintf(msgBuffer, "WiFi.status(): %d", networkStatus);
        debugMessage(msgBuffer, 2);
        if (networkStatus != NETWORK_CONNECTED) {
          *msgBuffer = 0;
          sprintf(msgBuffer, "Connection to \"%s\" failed", ssid);
          debugMessage(msgBuffer, 0);

          *msgBuffer = 0;
          sprintf(msgBuffer, "Retrying in  \"%d\" milliseconds", connectionTickTimeInterval);
          debugMessage(msgBuffer, 2);
          //changeConnectionState(CONNECTION_STATE_CONNECTING);
          return;
        } else {
          *msgBuffer = 0;
          sprintf(msgBuffer, "Connected to \"%s\"", ssid);
          debugMessage(msgBuffer, 2);
          changeConnectionState(CONNECTION_STATE_GETTIME);
          return;
        }
        break;
      case CONNECTION_STATE_GETTIME:
        debugMessage("Acquiring Time from Network", 3);
        unsigned long networkTime;
        networkTime = WiFi.getTime();
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
        networkStatus = WiFi.status();
        *msgBuffer = 0;
        sprintf(msgBuffer, "WiFi.status(): %d", networkStatus);
        debugMessage(msgBuffer, 2);
        if (networkStatus != WL_CONNECTED) {
          changeConnectionState(CONNECTION_STATE_DISCONNECTED);
          return;
        }
        *msgBuffer = 0;
        sprintf(msgBuffer, "Connected to \"%s\"", ssid);
        debugMessage(msgBuffer, 2);
        break;
      case CONNECTION_STATE_DISCONNECTED:
        //WiFi.disconnect();
        WiFi.end();

        *msgBuffer = 0;
        sprintf(msgBuffer, "DISC | WiFi.status(): %d", WiFi.status());
        debugMessage(msgBuffer, 1);
        *msgBuffer = 0;
        sprintf(msgBuffer, "Connection to \"%s\" lost.", ssid);
        debugMessage(msgBuffer, 0);
        debugMessage("Attempting reconnection", 1);
        changeConnectionState(CONNECTION_STATE_CONNECTING);
        //wifiClient.stop();
        break;
    }
    lastConnectionTickTime = now;
  }
}
