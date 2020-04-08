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

int ArduinoIoTCloudLPWAN::connect()
{
  _connection->connect();
  return 1;
}

bool ArduinoIoTCloudLPWAN::disconnect()
{
  _connection->disconnect();
  return true;
}

int ArduinoIoTCloudLPWAN::connected()
{
  return (_connection->getStatus() == NetworkConnectionState::CONNECTED) ? 1 : 0;
}

int ArduinoIoTCloudLPWAN::begin(ConnectionHandler& connection, bool retry)
{
  _connection = &connection;
  _retryEnable = retry;
  _thing.begin();
  return 1;
}

void ArduinoIoTCloudLPWAN::update()
{
  // Check if a primitive property wrapper is locally changed
  _thing.updateTimestampOnLocallyChangedProperties();

  ArduinoIoTConnectionStatus next_iot_status = _iot_status;

  /* Since we do not have a direct connection to the Arduino IoT Cloud servers
   * there is no such thing is a 'cloud connection state' since the LoRa
   * board connection state to the gateway is all the information we have.
   */
  NetworkConnectionState const net_con_state = checkPhyConnection();
  if     (net_con_state == NetworkConnectionState::CONNECTED)    { next_iot_status = ArduinoIoTConnectionStatus::CONNECTED;    execCloudEventCallback(ArduinoIoTCloudEvent::CONNECT); }
  else if(net_con_state == NetworkConnectionState::CONNECTING)   { next_iot_status = ArduinoIoTConnectionStatus::CONNECTING; }
  else if(net_con_state == NetworkConnectionState::DISCONNECTED) { next_iot_status = ArduinoIoTConnectionStatus::DISCONNECTED; execCloudEventCallback(ArduinoIoTCloudEvent::DISCONNECT); }

  if(next_iot_status != _iot_status)
  {
    printConnectionStatus(next_iot_status);
    _iot_status = next_iot_status;
  }

  if(net_con_state != NetworkConnectionState::CONNECTED) return;

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

void ArduinoIoTCloudLPWAN::printDebugInfo()
{
  Debug.print(DBG_INFO, "***** Arduino IoT Cloud LPWAN - configuration info *****");
  Debug.print(DBG_INFO, "Thing ID: %s", getThingId().c_str());
}

/******************************************************************************
 * PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

void ArduinoIoTCloudLPWAN::sendPropertiesToCloud()
{
  uint8_t data[CBOR_LORA_MSG_MAX_SIZE];
  int const length = _thing.encode(data, sizeof(data), true);
  if (length > 0) {
    writeProperties(data, length);
  }
}

int ArduinoIoTCloudLPWAN::writeProperties(const byte data[], int length)
{
  int retcode = _connection->write(data, length);
  int i = 0;
  while (_retryEnable && retcode < 0 && i < _maxNumRetry) {
    delay(_intervalRetry);
    retcode = _connection->write(data, length);
    i++;
  }

  return 1;
}

/******************************************************************************
 * EXTERN DEFINITION
 ******************************************************************************/

ArduinoIoTCloudLPWAN ArduinoCloud;

#endif