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
 * INCLUDE
 ******************************************************************************/

#include "ArduinoIoTCloud_Defines.h"

#ifdef HAS_LORA

#include<ArduinoIoTCloudLPWAN.h>

/******************************************************************************
   CONSTANTS
 ******************************************************************************/

static size_t const CBOR_LORA_MSG_MAX_SIZE = 255;

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

ArduinoIoTCloudLPWAN::ArduinoIoTCloudLPWAN()
: _retryEnable{false}
, _maxNumRetry{5}
, _intervalRetry{1000}
{

}

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

int ArduinoIoTCloudLPWAN::connect() {
  _connection->connect();
  const int state = _connection->getStatus() == NetworkConnectionState::INIT ? 1 : 0;
  return state;
}

bool ArduinoIoTCloudLPWAN::disconnect() {
  _connection->disconnect();
  return true;
}

int ArduinoIoTCloudLPWAN::connected() {
  int state = _connection->getStatus() == NetworkConnectionState::INIT ? 1 : 0;
  return state;
}

int ArduinoIoTCloudLPWAN::begin(ConnectionHandler& connection, bool retry) {
  _connection = &connection;
  _retryEnable = retry;
  _thing.begin();
  return 1;
}

void ArduinoIoTCloudLPWAN::update() {
  // Check if a primitive property wrapper is locally changed
  _thing.updateTimestampOnLocallyChangedProperties();

  if(checkPhyConnection()   != NetworkConnectionState::CONNECTED)     return;
  if(checkCloudConnection() != ArduinoIoTConnectionStatus::CONNECTED) return;

  if (_connection->available()) {
    uint8_t msgBuf[CBOR_LORA_MSG_MAX_SIZE];
    uint8_t i = 0;
    while (_connection->available()) {
      msgBuf[i++] = _connection->read();
    }

    _thing.decode(msgBuf, sizeof(msgBuf));
  }

  sendPropertiesToCloud();
  execCloudEventCallback(ArduinoIoTCloudEvent::SYNC);
}

void ArduinoIoTCloudLPWAN::printDebugInfo() {
  Debug.print(DBG_INFO, "***** Arduino IoT Cloud LPWAN- configuration info *****");
  Debug.print(DBG_INFO, "Device ID: %s", getDeviceId().c_str());
  Debug.print(DBG_INFO, "Thing ID: %s", getThingId().c_str());
}

/******************************************************************************
 * PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

void ArduinoIoTCloudLPWAN::sendPropertiesToCloud() {
  uint8_t data[CBOR_LORA_MSG_MAX_SIZE];
  int const length = _thing.encode(data, sizeof(data), true);
  if (length > 0) {
    writeProperties(data, length);
  }
}

int ArduinoIoTCloudLPWAN::writeProperties(const byte data[], int length) {
  int retcode = _connection->write(data, length);
  int i = 0;
  while (_retryEnable && retcode < 0 && i < _maxNumRetry) {
    delay(_intervalRetry);
    retcode = _connection->write(data, length);
    i++;
  }

  return 1;
}

NetworkConnectionState ArduinoIoTCloudLPWAN::checkPhyConnection()
{
  NetworkConnectionState const connect_state = _connection->check();

  if (_connection->check() != NetworkConnectionState::CONNECTED)
  {
    if (_iotStatus == ArduinoIoTConnectionStatus::CONNECTED)
    {
      disconnect();
    }
  }

  return connect_state;
}

ArduinoIoTConnectionStatus ArduinoIoTCloudLPWAN::checkCloudConnection()
{
  switch (_iotStatus) {
    case ArduinoIoTConnectionStatus::IDLE: {
        _iotStatus = ArduinoIoTConnectionStatus::CONNECTING;
        printConnectionStatus(_iotStatus);
      }
      break;
    case ArduinoIoTConnectionStatus::ERROR: {
        _iotStatus = ArduinoIoTConnectionStatus::RECONNECTING;
        printConnectionStatus(_iotStatus);
      }
      break;
    case ArduinoIoTConnectionStatus::CONNECTED: {
        if (_connection->getStatus() != NetworkConnectionState::CONNECTED) {
          _iotStatus = ArduinoIoTConnectionStatus::DISCONNECTED;
          printConnectionStatus(_iotStatus);
          execCloudEventCallback(ArduinoIoTCloudEvent::DISCONNECT);
        }
      }
      break;
    case ArduinoIoTConnectionStatus::DISCONNECTED: {
        _iotStatus = ArduinoIoTConnectionStatus::RECONNECTING;
        printConnectionStatus(_iotStatus);
      }
      break;
    case ArduinoIoTConnectionStatus::RECONNECTING: {
        int const ret_code = connect();
        Debug.print(DBG_INFO, "ArduinoCloud.reconnect()");
        if (ret_code == 1) {
          _iotStatus = ArduinoIoTConnectionStatus::IDLE;
        } else {
          _iotStatus = ArduinoIoTConnectionStatus::ERROR;
        }

      }
      break;
    case ArduinoIoTConnectionStatus::CONNECTING: {
        NetworkConnectionState net_status = _connection->getStatus();
        Debug.print(DBG_VERBOSE, "ArduinoCloud.connect(): %d", net_status);
        if (net_status == NetworkConnectionState::CONNECTED) {
          _iotStatus = ArduinoIoTConnectionStatus::CONNECTED;
          printConnectionStatus(_iotStatus);
          execCloudEventCallback(ArduinoIoTCloudEvent::CONNECT);
        }

      }
      break;
  }

  return _iotStatus;
}

/******************************************************************************
 * EXTERN DEFINITION
 ******************************************************************************/

ArduinoIoTCloudLPWAN ArduinoCloud;

#endif