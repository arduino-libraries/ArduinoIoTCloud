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
#include "ArduinoIoTCloud_Defines.h"
#ifdef HAS_LORA

#include<ArduinoIoTCloudLPWAN.h>

#ifdef ARDUINO_ARCH_SAMD
  #include <RTCZero.h>
  RTCZero rtc;
#endif

ArduinoIoTCloudLPWAN::ArduinoIoTCloudLPWAN() :
  _connection(NULL) {}

ArduinoIoTCloudLPWAN::~ArduinoIoTCloudLPWAN() {
}

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

int ArduinoIoTCloudLPWAN::begin(LPWANConnectionHandler& connection, bool retry) {
  _connection = &connection;
  _connection->init();
  _retryEnable = retry;
  _maxNumRetry = 5;
  _intervalRetry = 1000;
  Thing.begin();
  return 1;
}

void ArduinoIoTCloudLPWAN::update(CallbackFunc onSyncCompleteCallback) {
  // Check if a primitive property wrapper is locally changed
  Thing.updateTimestampOnLocallyChangedProperties();

  connectionCheck();

  if (_iotStatus != ArduinoIoTConnectionStatus::CONNECTED) {
    return;
  }

  if (_connection->available()) {
    uint8_t msgBuf[DEFAULT_CBOR_LORA_MSG_SIZE];
    uint8_t i = 0;
    while (_connection->available()) {
      msgBuf[i++] = _connection->read();
    }

    Thing.decode(msgBuf, sizeof(msgBuf));
  }




  sendPropertiesToCloud();


  if (onSyncCompleteCallback != NULL) {
    (*onSyncCompleteCallback)();
  }
  execCloudEventCallback(_on_sync_event_callback, 0 /* callback_arg */);

}

void ArduinoIoTCloudLPWAN::connectionCheck() {
  if (_connection != NULL) {

    _connection->check();

    if (_connection->getStatus() != NetworkConnectionState::CONNECTED) {
      if (_iotStatus == ArduinoIoTConnectionStatus::CONNECTED) {
        _iotStatus = ArduinoIoTConnectionStatus::DISCONNECTED;
        printConnectionStatus(_iotStatus);
      }
      return;
    }
  }

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
          execCloudEventCallback(_on_disconnect_event_callback, 0 /* callback_arg - e.g. could be error code casted to void * */);
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
          execCloudEventCallback(_on_connect_event_callback, 0 /* callback_arg */);
        }

      }
      break;
  }
}

void ArduinoIoTCloudLPWAN::printDebugInfo() {
  Debug.print(DBG_INFO, "***** Arduino IoT Cloud LPWAN- configuration info *****");
  Debug.print(DBG_INFO, "Device ID: %s", getDeviceId().c_str());
  Debug.print(DBG_INFO, "Thing ID: %s", getThingId().c_str());

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

int ArduinoIoTCloudLPWAN::writeStdout(const byte data[], int length) {
  int retcode = _connection->write(data, length);
  int i = 0;
  while (_retryEnable && retcode < 0 && i < _maxNumRetry) {
    delay(_intervalRetry);
    retcode = _connection->write(data, length);
    i++;
  }

  return 1;
}

int ArduinoIoTCloudLPWAN::writeShadowOut(const byte data[], int length) {
  int retcode = _connection->write(data, length);
  int i = 0;
  while (_retryEnable && retcode < 0 && i < _maxNumRetry) {
    delay(_intervalRetry);
    retcode = _connection->write(data, length);
    i++;
  }
  return 1;
}

void ArduinoIoTCloudLPWAN::sendPropertiesToCloud() {
  uint8_t data[DEFAULT_CBOR_LORA_MSG_SIZE];
  int const length = Thing.encode(data, sizeof(data), true);
  if (length > 0) {
    writeProperties(data, length);
  }
}

ArduinoIoTCloudLPWAN ArduinoCloud;

#endif