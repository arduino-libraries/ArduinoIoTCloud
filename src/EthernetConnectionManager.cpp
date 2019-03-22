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

#include "EthernetConnectionManager.h"

#ifdef BOARD_HAS_ETHERNET /* Only compile if the board has ethernet */

/******************************************************************************
 * CONSTANTS
 ******************************************************************************/

static const unsigned long NETWORK_CONNECTION_INTERVAL = 30000;

/******************************************************************************
 * CTOR
 ******************************************************************************/

EthConnectionManager::EthConnectionManager(uint8_t * mac, int const ss_pin) :
  mac(mac),
  ss_pin(ss_pin),
  lastConnectionTickTime(millis()),
  connectionTickTimeInterval(CHECK_INTERVAL_IDLE) {
}

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

void EthConnectionManager::init() {
}

unsigned long EthConnectionManager::getTime() {
  //handled by fallback manager
  return lastValidTimestamp + 1;
}

void EthConnectionManager::check() {
  unsigned long const now = millis();
  int networkStatus = 0;
  if (now - lastConnectionTickTime > connectionTickTimeInterval) {
    switch (netConnectionState) {
      case CONNECTION_STATE_INIT:
        if (ss_pin == -1) {
          networkStatus = Ethernet.begin(mac);
        } else {
          networkStatus = Ethernet.begin(mac, ss_pin);
        }
        networkStatus = Ethernet.hardwareStatus();
        debugMessage(DebugLevel::Info, "Eth hardware status(): %d", networkStatus);
        if (networkStatus == EthernetNoHardware) {
          debugMessage(DebugLevel::Error, "No Ethernet chip connected");
          // don't continue:
          changeConnectionState(CONNECTION_STATE_ERROR);
          lastConnectionTickTime = now;
          return;
        }
        networkStatus = Ethernet.linkStatus();
        debugMessage(DebugLevel::Info, "Eth link status(): %d", networkStatus);
        if (networkStatus == LinkOFF) {
          debugMessage(DebugLevel::Error, "Failed to configure Ethernet via dhcp");
          // don't continue:
          changeConnectionState(CONNECTION_STATE_ERROR);
          lastConnectionTickTime = now;
          return;
        }
        debugMessage(DebugLevel::Error, "Ethernet shield recognized: ID", Ethernet.hardwareStatus());
        changeConnectionState(CONNECTION_STATE_CONNECTING);
        break;
      case CONNECTION_STATE_CONNECTING:
        debugMessage(DebugLevel::Info, "Connecting via dhcp");
        if (ss_pin == -1) {
          networkStatus = Ethernet.begin(mac);
        } else {
          networkStatus = Ethernet.begin(mac, ss_pin);
        }
        debugMessage(DebugLevel::Info, "Ethernet.status(): %d", networkStatus);
        if (networkStatus == 0) {
          debugMessage(DebugLevel::Error, "Connection failed");
          debugMessage(DebugLevel::Info, "Retrying in  \"%d\" milliseconds", connectionTickTimeInterval);
          //changeConnectionState(CONNECTION_STATE_CONNECTING);
          return;
        } else {
          debugMessage(DebugLevel::Info, "Connected!");
          changeConnectionState(CONNECTION_STATE_GETTIME);
          return;
        }
        break;
      case CONNECTION_STATE_GETTIME:
        debugMessage(DebugLevel::Debug, "Acquiring Time from Network");
        unsigned long networkTime;
        networkTime = getTime();
        debugMessage(DebugLevel::Debug, "Network Time: %u", networkTime);
        if(networkTime > lastValidTimestamp){
          lastValidTimestamp = networkTime;
          changeConnectionState(CONNECTION_STATE_CONNECTED);
        }
        break;
      case CONNECTION_STATE_CONNECTED:
        // keep testing connection
        Ethernet.maintain();
        networkStatus = Ethernet.linkStatus();
        debugMessage(DebugLevel::Verbose, "Eth link status(): %d", networkStatus);
        if (networkStatus != LinkON) {
          changeConnectionState(CONNECTION_STATE_DISCONNECTED);
          return;
        }
        debugMessage(DebugLevel::Info, "Connected");
        break;
      case CONNECTION_STATE_DISCONNECTED:
        debugMessage(DebugLevel::Error, "Connection lost.");
        debugMessage(DebugLevel::Info, "Attempting reconnection");
        changeConnectionState(CONNECTION_STATE_CONNECTING);
        //wifiClient.stop();
        break;
    }
    lastConnectionTickTime = now;
  }
}

/******************************************************************************
 * PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

void EthConnectionManager::changeConnectionState(NetworkConnectionState _newState) {
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

#endif /* #ifdef BOARD_HAS_ETHERNET */
