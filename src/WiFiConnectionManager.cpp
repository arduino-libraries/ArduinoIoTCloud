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

#include "WiFiConnectionManager.h"

#ifdef BOARD_HAS_WIFI /* Only compile if the board has WiFi */

/******************************************************************************
   CONSTANTS
 ******************************************************************************/

static const unsigned long NETWORK_CONNECTION_INTERVAL = 30000;

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

WiFiConnectionManager::WiFiConnectionManager(const char *ssid, const char *pass) :
  ssid(ssid),
  pass(pass),
  lastConnectionTickTime(millis()),
  connectionTickTimeInterval(CHECK_INTERVAL_IDLE) {
}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

void WiFiConnectionManager::init() {
}

unsigned long WiFiConnectionManager::getTime() {
  return WiFi.getTime();
}

void WiFiConnectionManager::check() {
  unsigned long const now = millis();
  int networkStatus = 0;
  if (now - lastConnectionTickTime > connectionTickTimeInterval) {
    switch (netConnectionState) {
      case NetworkConnectionState::INIT: {
          networkStatus = WiFi.status();
          debugMessage(DebugLevel::Info, "WiFi.status(): %d", networkStatus);
          if (networkStatus == NETWORK_HARDWARE_ERROR) {
            // NO FURTHER ACTION WILL FOLLOW THIS
            changeConnectionState(NetworkConnectionState::ERROR);
            lastConnectionTickTime = now;
            return;
          }
          debugMessage(DebugLevel::Error, "Current WiFi Firmware: %s", WiFi.firmwareVersion());
          if (strcmp(WiFi.firmwareVersion(), WIFI_FIRMWARE_VERSION_REQUIRED) < 0) {
            debugMessage(DebugLevel::Error, "Latest WiFi Firmware: %s", WIFI_FIRMWARE_VERSION_REQUIRED);
            debugMessage(DebugLevel::Error, "Please update to the latest version for best performance.");
            delay(5000);
          }
          changeConnectionState(NetworkConnectionState::CONNECTING);
        }
        break;
      case NetworkConnectionState::CONNECTING: {
          networkStatus = WiFi.begin(ssid, pass);
          debugMessage(DebugLevel::Verbose, "WiFi.status(): %d", networkStatus);
          if (networkStatus != NETWORK_CONNECTED) {
            debugMessage(DebugLevel::Error, "Connection to \"%s\" failed", ssid);
            debugMessage(DebugLevel::Info, "Retrying in  \"%d\" milliseconds", connectionTickTimeInterval);
            //changeConnectionState(NetworkConnectionState::CONNECTING);
            return;
          } else {
            debugMessage(DebugLevel::Info, "Connected to \"%s\"", ssid);
            changeConnectionState(NetworkConnectionState::CONNECTED);
            return;
          }
        }
        break;
      case NetworkConnectionState::CONNECTED: {
          // keep testing connection
          networkStatus = WiFi.status();
          debugMessage(DebugLevel::Verbose, "WiFi.status(): %d", networkStatus);
          if (networkStatus != WL_CONNECTED) {
            changeConnectionState(NetworkConnectionState::DISCONNECTED);
            return;
          }
          debugMessage(DebugLevel::Verbose, "Connected to \"%s\"", ssid);
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
          WiFi.end();
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

void WiFiConnectionManager::changeConnectionState(NetworkConnectionState const newState) {
  connectionTickTimeInterval = CHECK_INTERVAL_INIT;
  netConnectionState = newState;

  switch (netConnectionState) {
    case NetworkConnectionState::INIT: {
        connectionTickTimeInterval = CHECK_INTERVAL_INIT;
      }
      break;
    case NetworkConnectionState::CONNECTING: {
        debugMessage(DebugLevel::Info, "Connecting to \"%s\"", ssid);
        connectionTickTimeInterval = CHECK_INTERVAL_CONNECTING;
      }
      break;
    case NetworkConnectionState::CONNECTED: {
        connectionTickTimeInterval = CHECK_INTERVAL_CONNECTED;
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
        debugMessage(DebugLevel::Verbose, "WiFi.status(): %d", WiFi.status());
        debugMessage(DebugLevel::Error, "Connection to \"%s\" lost.", ssid);
        debugMessage(DebugLevel::Error, "Attempting reconnection");
        connectionTickTimeInterval = CHECK_INTERVAL_DISCONNECTED;
      }
      break;
    case NetworkConnectionState::ERROR: {
        debugMessage(DebugLevel::Error, "WiFi Hardware failure.\nMake sure you are using a WiFi enabled board/shield.");
        debugMessage(DebugLevel::Error, "Then reset and retry.");
      }
      break;
  }

  lastConnectionTickTime = millis();
}

#endif /* #ifdef BOARD_HAS_WIFI */
