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

#include <AIoTC_Config.h>

#ifdef HAS_TCP
#include <ArduinoIoTCloudTCP.h>
#include <ArduinoIoTCloudTCPThing.h>
#endif

#include <algorithm>
#include "cbor/CBOREncoder.h"

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

ArduinoIoTCloudTCPThing::ArduinoIoTCloudTCPThing()
: _state{State::WaitDeviceConfig}
, _next_connection_attempt_tick{0}
, _last_connection_attempt_cnt{0}
, _next_device_subscribe_attempt_tick{0}
, _last_device_subscribe_cnt{0}
, _last_sync_request_tick{0}
, _last_sync_request_cnt{0}
, _last_subscribe_request_tick{0}
, _last_subscribe_request_cnt{0}
, _mqtt_data_buf{0}
, _mqtt_data_len{0}
, _mqtt_data_request_retransmit{false}
, _mqttClient{nullptr}
, _deviceTopicOut("")
, _deviceTopicIn("")
, _shadowTopicOut("")
, _shadowTopicIn("")
, _dataTopicOut("")
, _dataTopicIn("")
, _deviceSubscribedToThing{false}
{

}

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

int ArduinoIoTCloudTCPThing::begin(MqttClient& mqttClient, PropertyContainer & thing_property_container, void (*callback)(ArduinoIoTCloudEvent))
{
  _mqttClient = mqttClient;
  _thing_property_container = thing_property_container;
  _execCloudEventCallback = callback;
  return 1;
}


void ArduinoIoTCloudTCP::update()
{
  /* Run through the state machine. */
  State next_state = _state;
  switch (_state)
  {
  case State::WaitDeviceConfig:     next_state = handle_WaitDeviceConfig();     break;
  case State::CheckDeviceConfig:    next_state = handle_CheckDeviceConfig();    break;
  case State::SubscribeThingTopics: next_state = handle_SubscribeThingTopics(); break;
  case State::RequestLastValues:    next_state = handle_RequestLastValues();    break;
  case State::Connected:            next_state = handle_Connected();            break;
  case State::Disconnect:           next_state = handle_Disconnect();           break;
  }
  _state = next_state;
}

int ArduinoIoTCloudTCP::connected()
{
  return _mqttClient.connected();
}

/******************************************************************************
 * PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_WaitDeviceConfig()
{
  if (!_mqttClient.connected())
  {
    return State::Disconnect;
  }

  // If the thing id is not set, we need to wait for it
  if (deviceNotAttached())
  {
    return State::WaitDeviceConfig;
  }

  if (getThingIdOutdatedFlag())
  {
    return State::CheckDeviceConfig;
  }

  return State::CheckDeviceConfig;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_CheckDeviceConfig()
{
  if (!_mqttClient.connected())
  {
    return State::Disconnect;
  }

  if(_deviceSubscribedToThing == true)
  {
    /* Unsubscribe from old things topics and go on with a new subscription */
    _mqttClient.unsubscribe(_shadowTopicIn);
    _mqttClient.unsubscribe(_dataTopicIn);
    _deviceSubscribedToThing = false;
    DEBUG_INFO("Disconnected from Arduino IoT Cloud");
    _execCloudEventCallback(ArduinoIoTCloudEvent::DISCONNECT);
  }

  updateThingTopics();

  if (deviceNotAttached())
  {
    return State::WaitDeviceConfig;
  }

  DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s Device attached to a new valid Thing %s", __FUNCTION__, getThingId().c_str());

  return State::SubscribeThingTopics;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_SubscribeThingTopics()
{
  if (!_mqttClient.connected())
  {
    return State::Disconnect;
  }

  if (getThingIdOutdatedFlag())
  {
    return State::CheckDeviceConfig;
  }

  unsigned long const now = millis();
  bool const is_subscribe_retry_delay_expired = (now - _last_subscribe_request_tick) > AIOT_CONFIG_THING_TOPICS_SUBSCRIBE_RETRY_DELAY_ms;
  bool const is_first_subscribe_request = (_last_subscribe_request_cnt == 0);

  if (!is_first_subscribe_request && !is_subscribe_retry_delay_expired)
  {
    return State::SubscribeThingTopics;
  }

  if (_last_subscribe_request_cnt > AIOT_CONFIG_THING_TOPICS_SUBSCRIBE_MAX_RETRY_CNT)
  {
    _last_subscribe_request_cnt = 0;
    _last_subscribe_request_tick = 0;
    _mqttClient.stop();
    _execCloudEventCallback(ArduinoIoTCloudEvent::DISCONNECT);
    return State::ConnectPhy;
  }

  _last_subscribe_request_tick = now;
  _last_subscribe_request_cnt++;

  if (!_mqttClient.subscribe(_dataTopicIn))
  {
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s could not subscribe to %s", __FUNCTION__, _dataTopicIn.c_str());
    DEBUG_ERROR("Check your thing configuration, and press the reset button on your board.");
    return State::SubscribeThingTopics;
  }

  if (!_mqttClient.subscribe(_shadowTopicIn))
  {
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s could not subscribe to %s", __FUNCTION__, _shadowTopicIn.c_str());
    DEBUG_ERROR("Check your thing configuration, and press the reset button on your board.");
    return State::SubscribeThingTopics;
  }

  DEBUG_INFO("Connected to Arduino IoT Cloud");
  DEBUG_INFO("Thing ID: %s", getThingId().c_str());
  _execCloudEventCallback(ArduinoIoTCloudEvent::CONNECT);
  _deviceSubscribedToThing = true;

  /*Add retry wait time otherwise we are trying to reconnect every 250 ms...*/
  return State::RequestLastValues;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_RequestLastValues()
{
  if (!_mqttClient.connected())
  {
    return State::Disconnect;
  }

  if (getThingIdOutdatedFlag())
  {
    return State::CheckDeviceConfig;
  }

  /* Check whether or not we need to send a new request. */
  unsigned long const now = millis();
  bool const is_sync_request_timeout = (now - _last_sync_request_tick) > AIOT_CONFIG_TIMEOUT_FOR_LASTVALUES_SYNC_ms;
  bool const is_first_sync_request = (_last_sync_request_cnt == 0);
  if (is_first_sync_request || is_sync_request_timeout)
  {
    DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s [%d] last values requested", __FUNCTION__, now);
    requestLastValue();
    _last_sync_request_tick = now;
    /* Track the number of times a get-last-values request was sent to the cloud.
     * If no data is received within a certain number of retry-requests it's a better
     * strategy to disconnect and re-establish connection from the ground up.
     */
    _last_sync_request_cnt++;
    if (_last_sync_request_cnt > AIOT_CONFIG_LASTVALUES_SYNC_MAX_RETRY_CNT)
    {
      _last_sync_request_cnt = 0;
      _last_sync_request_tick = 0;
      _mqttClient.stop();
      _execCloudEventCallback(ArduinoIoTCloudEvent::DISCONNECT);
      return State::ConnectPhy;
    }
  }

  return State::RequestLastValues;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_Connected()
{
  if (!_mqttClient.connected())
  {
    /* The last message was definitely lost, trigger a retransmit. */
    _mqtt_data_request_retransmit = true;
    return State::Disconnect;
  }
  /* We are connected so let's to our stuff here. */
  else
  {
    if (getThingIdOutdatedFlag())
    {
      return State::CheckDeviceConfig;
    }

    /* Check if a primitive property wrapper is locally changed.
    * This function requires an existing time service which in
    * turn requires an established connection. Not having that
    * leads to a wrong time set in the time service which inhibits
    * the connection from being established due to a wrong data
    * in the reconstructed certificate.
    */
    updateTimestampOnLocallyChangedProperties(*_thing_property_container);

    /* Retransmit data in case there was a lost transaction due
    * to phy layer or MQTT connectivity loss.
    */
    if(_mqtt_data_request_retransmit) {
      write(_dataTopicOut, _mqtt_data_buf, _mqtt_data_len);
      _mqtt_data_request_retransmit = false;
    }

    /* Check if any properties need encoding and send them to
    * the cloud if necessary.
    */
    sendThingPropertiesToCloud();

    unsigned long const internal_posix_time = _time_service.getTime();
    if(internal_posix_time < _tz_dst_until) {
      return State::Connected;
    } else {
      return State::RequestLastValues;
    }
  }
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_Disconnect()
{
  DEBUG_ERROR("ArduinoIoTCloudTCP::%s MQTT client connection lost", __FUNCTION__);
  _mqttClient.stop();
  _execCloudEventCallback(ArduinoIoTCloudEvent::DISCONNECT);
  return State::WaitDeviceConfig
}

void ArduinoIoTCloudTCP::onMessage(int length)
{
  ArduinoCloud.handleMessage(length);
}

void ArduinoIoTCloudTCP::sendPropertyContainerToCloud(String const topic, PropertyContainer & property_container, unsigned int & current_property_index)
{
  int bytes_encoded = 0;
  uint8_t data[MQTT_TRANSMIT_BUFFER_SIZE];

  if (CBOREncoder::encode(property_container, data, sizeof(data), bytes_encoded, current_property_index, false) == CborNoError)
    if (bytes_encoded > 0)
    {
      /* If properties have been encoded store them in the back-up buffer
       * in order to allow retransmission in case of failure.
       */
      _mqtt_data_len = bytes_encoded;
      memcpy(_mqtt_data_buf, data, _mqtt_data_len);
      /* Transmit the properties to the MQTT broker */
      write(topic, _mqtt_data_buf, _mqtt_data_len);
    }
}

void ArduinoIoTCloudTCP::sendThingPropertiesToCloud()
{
  sendPropertyContainerToCloud(_dataTopicOut, *_thing_property_container, _last_checked_property_index);
}

void ArduinoIoTCloudTCP::requestLastValue()
{
  // Send the getLastValues CBOR message to the cloud
  // [{0: "r:m", 3: "getLastValues"}] = 81 A2 00 63 72 3A 6D 03 6D 67 65 74 4C 61 73 74 56 61 6C 75 65 73
  // Use http://cbor.me to easily generate CBOR encoding
  const uint8_t CBOR_REQUEST_LAST_VALUE_MSG[] = { 0x81, 0xA2, 0x00, 0x63, 0x72, 0x3A, 0x6D, 0x03, 0x6D, 0x67, 0x65, 0x74, 0x4C, 0x61, 0x73, 0x74, 0x56, 0x61, 0x6C, 0x75, 0x65, 0x73 };
  write(_shadowTopicOut, CBOR_REQUEST_LAST_VALUE_MSG, sizeof(CBOR_REQUEST_LAST_VALUE_MSG));
}

int ArduinoIoTCloudTCP::write(String const topic, byte const data[], int const length)
{
  if (_mqttClient.beginMessage(topic, length, false, 0)) {
    if (_mqttClient.write(data, length)) {
      if (_mqttClient.endMessage()) {
        return 1;
      }
    }
  }
  return 0;
}

void ArduinoIoTCloudTCP::updateThingTopics()
{
  _shadowTopicOut = getTopic_shadowout();
  _shadowTopicIn  = getTopic_shadowin();
  _dataTopicOut   = getTopic_dataout();
  _dataTopicIn    = getTopic_datain();

  clrThingIdOutdatedFlag();
}

#endif