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

#include "EthernetConnectionManager.h"

#ifdef BOARD_HAS_ETHERNET /* Only compile if the board has ethernet */

/******************************************************************************
   CONSTANTS
 ******************************************************************************/

static const unsigned long NETWORK_CONNECTION_INTERVAL = 30000;

/******************************************************************************
   CTOR
 ******************************************************************************/

EthConnectionManager::EthConnectionManager(uint8_t * mac, int const ss_pin) :
  mac(mac),
  ss_pin(ss_pin),
  lastConnectionTickTime(millis()),
  connectionTickTimeInterval(CHECK_INTERVAL_IDLE) {
}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
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
      case NetworkConnectionState::INIT: {
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
            changeConnectionState(NetworkConnectionState::ERROR);
            lastConnectionTickTime = now;
            return;
          }
          networkStatus = Ethernet.linkStatus();
          debugMessage(DebugLevel::Info, "Eth link status(): %d", networkStatus);
          if (networkStatus == LinkOFF) {
            debugMessage(DebugLevel::Error, "Failed to configure Ethernet via dhcp");
            // don't continue:
            changeConnectionState(NetworkConnectionState::ERROR);
            lastConnectionTickTime = now;
            return;
          }
          debugMessage(DebugLevel::Error, "Ethernet shield recognized: ID", Ethernet.hardwareStatus());
          changeConnectionState(NetworkConnectionState::CONNECTING);
        }
        break;
      case NetworkConnectionState::CONNECTING: {
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
            //changeConnectionState(NetworkConnectionState::CONNECTING);
            return;
          } else {
            debugMessage(DebugLevel::Info, "Connected!");
            changeConnectionState(NetworkConnectionState::GETTIME);
            return;
          }
        }
        break;
      case NetworkConnectionState::GETTIME: {
          debugMessage(DebugLevel::Debug, "Acquiring Time from Network");
          unsigned long networkTime;
          networkTime = getTime();
          debugMessage(DebugLevel::Debug, "Network Time: %u", networkTime);
          if (networkTime > lastValidTimestamp) {
            lastValidTimestamp = networkTime;
            changeConnectionState(NetworkConnectionState::CONNECTED);
          }
        }
        break;
      case NetworkConnectionState::CONNECTED: {
          // keep testing connection
          Ethernet.maintain();
          networkStatus = Ethernet.linkStatus();
          debugMessage(DebugLevel::Verbose, "Eth link status(): %d", networkStatus);
          if (networkStatus != LinkON) {
            changeConnectionState(NetworkConnectionState::DISCONNECTED);
            return;
          }
          debugMessage(DebugLevel::Info, "Connected");
        }
        break;
      case NetworkConnectionState::DISCONNECTING: {
          /* Do nothing */
        }
        break;
      case NetworkConnectionState::DISCONNECTED: {
          debugMessage(DebugLevel::Error, "Connection lost.");
          debugMessage(DebugLevel::Info, "Attempting reconnection");
          changeConnectionState(NetworkConnectionState::CONNECTING);
          //wifiClient.stop();
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

void EthConnectionManager::changeConnectionState(NetworkConnectionState const newState) {
  netConnectionState = newState;
  switch (netConnectionState) {
    case NetworkConnectionState::INIT:          connectionTickTimeInterval = CHECK_INTERVAL_INIT;          break;
    case NetworkConnectionState::CONNECTING:    connectionTickTimeInterval = CHECK_INTERVAL_CONNECTING;    break;
    case NetworkConnectionState::GETTIME:       connectionTickTimeInterval = CHECK_INTERVAL_GETTIME;       break;
    case NetworkConnectionState::CONNECTED:     connectionTickTimeInterval = CHECK_INTERVAL_CONNECTED;     break;
    case NetworkConnectionState::DISCONNECTED:  connectionTickTimeInterval = CHECK_INTERVAL_DISCONNECTED;  break;
    case NetworkConnectionState::ERROR:         connectionTickTimeInterval = CHECK_INTERVAL_ERROR;         break;
    default:                                    connectionTickTimeInterval = CHECK_INTERVAL_IDLE;          break;
  }
  lastConnectionTickTime = millis();
}

#endif /* #ifdef BOARD_HAS_ETHERNET */
