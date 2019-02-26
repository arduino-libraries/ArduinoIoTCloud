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
#include <WiFiUdp.h>

class WiFiConnectionManager : public ConnectionManager {
public:
  WiFiConnectionManager(const char *ssid, const char *pass);

  virtual unsigned long getTime();
  virtual void init();
  virtual void check();
  virtual Client &getClient() { return wifiClient; };
  virtual UDP &getUDP() { return udp; };

  WiFiUDP udp;
  
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

  const char *ssid, *pass;
  unsigned long lastConnectionTickTime;
  unsigned long getTimeRetries;

  WiFiClient wifiClient;
  int connectionTickTimeInterval;
};

static const unsigned long NETWORK_CONNECTION_INTERVAL = 30000;

WiFiConnectionManager::WiFiConnectionManager(const char *ssid, const char *pass) :
  ssid(ssid), pass(pass),
  lastConnectionTickTime(millis()),
  connectionTickTimeInterval(CHECK_INTERVAL_IDLE),
  getTimeRetries(MAX_GETTIME_RETRIES) {
}

unsigned long WiFiConnectionManager::getTime() {
  return WiFi.getTime();
}

void WiFiConnectionManager::init() {
}

void WiFiConnectionManager::changeConnectionState(NetworkConnectionState _newState) {
  char msgBuffer[120];
  int newInterval = CHECK_INTERVAL_INIT;
  switch (_newState) {
    case CONNECTION_STATE_INIT:
      newInterval = CHECK_INTERVAL_INIT;
      break;
    case CONNECTION_STATE_CONNECTING:
      sprintf(msgBuffer, "Connecting to \"%s\"", ssid);
      debugMessage(msgBuffer, 2);
      newInterval = CHECK_INTERVAL_CONNECTING;
      break;
    case CONNECTION_STATE_GETTIME:
      newInterval = CHECK_INTERVAL_GETTIME;
      debugMessage("Acquiring Time from Network", 3);
      getTimeRetries = MAX_GETTIME_RETRIES;
      break;
    case CONNECTION_STATE_CONNECTED:
      newInterval = CHECK_INTERVAL_CONNECTED;
      break;
    case CONNECTION_STATE_DISCONNECTED:
      sprintf(msgBuffer, "WiFi.status(): %d", WiFi.status());
      debugMessage(msgBuffer, 4);
      sprintf(msgBuffer, "Connection to \"%s\" lost.", ssid);
      debugMessage(msgBuffer, 0);
      debugMessage("Attempting reconnection", 0);
      newInterval = CHECK_INTERVAL_DISCONNECTED;
      break;
    case CONNECTION_STATE_ERROR:
      debugMessage("WiFi Hardware failure.\nMake sure you are using a WiFi enabled board/shield.", 0);
      debugMessage("Then reset and retry.", 0);
      break;
  }
  connectionTickTimeInterval = newInterval;
  lastConnectionTickTime = millis();
  netConnectionState = _newState;
}

void WiFiConnectionManager::check() {
  char msgBuffer[120];
  unsigned long const now = millis();
  int networkStatus = 0;
  if (now - lastConnectionTickTime > connectionTickTimeInterval) {
    switch (netConnectionState) {
      case CONNECTION_STATE_INIT:
        networkStatus = WiFi.status();
        sprintf(msgBuffer, "WiFi.status(): %d", networkStatus);
        debugMessage(msgBuffer, 2);
        if (networkStatus == NETWORK_HARDWARE_ERROR) {
          // NO FURTHER ACTION WILL FOLLOW THIS
          changeConnectionState(CONNECTION_STATE_ERROR);
          lastConnectionTickTime = now;
          return;
        }
        sprintf(msgBuffer, "Current WiFi Firmware: %s", WiFi.firmwareVersion());
        debugMessage(msgBuffer, 0);
        if(WiFi.firmwareVersion() < WIFI_FIRMWARE_VERSION_REQUIRED){
          sprintf(msgBuffer, "Latest WiFi Firmware: %s", WIFI_FIRMWARE_VERSION_REQUIRED);
          debugMessage(msgBuffer, 0);
          debugMessage("Please update to the latest version for best performance.", 0);
          delay(5000);
        }
        changeConnectionState(CONNECTION_STATE_CONNECTING);
        break;
      case CONNECTION_STATE_CONNECTING:
        networkStatus = WiFi.begin(ssid, pass);
        sprintf(msgBuffer, "WiFi.status(): %d", networkStatus);
        debugMessage(msgBuffer, 4);
        if (networkStatus != NETWORK_CONNECTED) {
          sprintf(msgBuffer, "Connection to \"%s\" failed", ssid);
          debugMessage(msgBuffer, 0);
          sprintf(msgBuffer, "Retrying in  \"%d\" milliseconds", connectionTickTimeInterval);
          debugMessage(msgBuffer, 2);
          //changeConnectionState(CONNECTION_STATE_CONNECTING);
          return;
        } else {
          sprintf(msgBuffer, "Connected to \"%s\"", ssid);
          debugMessage(msgBuffer, 2);
          changeConnectionState(CONNECTION_STATE_GETTIME);
          return;
        }
        break;
      case CONNECTION_STATE_GETTIME:
        unsigned long networkTime;
        networkTime = getTime();
        debugMessage(".", 3, false, false);
        if(networkTime > lastValidTimestamp){
          debugMessage("", 3, false, true);
          lastValidTimestamp = networkTime;
          sprintf(msgBuffer, "Network Time: %u", networkTime);
          debugMessage(msgBuffer, 3);
          changeConnectionState(CONNECTION_STATE_CONNECTED);
        } else if (WiFi.status() != WL_CONNECTED) {
           changeConnectionState(CONNECTION_STATE_DISCONNECTED);
        } else if (!getTimeRetries--) {
           changeConnectionState(CONNECTION_STATE_DISCONNECTED);
        } 
        break;
      case CONNECTION_STATE_CONNECTED:
        // keep testing connection
        networkStatus = WiFi.status();
        sprintf(msgBuffer, "WiFi.status(): %d", networkStatus);
        debugMessage(msgBuffer, 4);
        if (networkStatus != WL_CONNECTED) {
          changeConnectionState(CONNECTION_STATE_DISCONNECTED);
          return;
        }
        sprintf(msgBuffer, "Connected to \"%s\"", ssid);
        debugMessage(msgBuffer, 4);
        break;
      case CONNECTION_STATE_DISCONNECTED:
        WiFi.end();
        changeConnectionState(CONNECTION_STATE_CONNECTING);
        break;
    }
    lastConnectionTickTime = now;
  }
}
