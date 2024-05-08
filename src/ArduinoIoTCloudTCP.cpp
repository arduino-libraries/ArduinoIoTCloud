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

#if defined(BOARD_HAS_SECRET_KEY)
  #include "tls/AIoTCUPCert.h"
#endif

#ifdef BOARD_HAS_ECCX08
  #include "tls/BearSSLTrustAnchors.h"
#endif

#if defined(BOARD_HAS_SE050) || defined(BOARD_HAS_SOFTSE)
  #include "tls/AIoTCSSCert.h"
#endif

#if OTA_ENABLED
  #include "utility/ota/OTA.h"
#endif

#include <algorithm>
#include "cbor/CBOREncoder.h"
#include "utility/watchdog/Watchdog.h"

/******************************************************************************
   LOCAL MODULE FUNCTIONS
 ******************************************************************************/

unsigned long getTime()
{
  return ArduinoCloud.getInternalTime();
}

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

ArduinoIoTCloudTCP::ArduinoIoTCloudTCP()
: _state{State::ConnectPhy}
, _tz_offset{0}
, _tz_offset_property{nullptr}
, _tz_dst_until{0}
, _tz_dst_until_property{nullptr}
, _next_connection_attempt_tick{0}
, _last_connection_attempt_cnt{0}
, _next_device_subscribe_attempt_tick{0}
, _last_device_subscribe_cnt{0}
, _next_thing_subscribe_attempt_tick{0}
, _last_thing_subscribe_attempt_cnt{0}
, _next_sync_attempt_tick{0}
, _last_sync_attempt_cnt{0}
, _mqtt_data_buf{0}
, _mqtt_data_len{0}
, _mqtt_data_request_retransmit{false}
#ifdef BOARD_HAS_ECCX08
, _sslClient(nullptr, ArduinoIoTCloudTrustAnchor, ArduinoIoTCloudTrustAnchor_NUM, getTime)
#endif
#ifdef BOARD_HAS_SECRET_KEY
, _password("")
#endif
, _mqttClient{nullptr}
, _deviceTopicOut("")
, _deviceTopicIn("")
, _shadowTopicOut("")
, _shadowTopicIn("")
, _dataTopicOut("")
, _dataTopicIn("")
#if OTA_ENABLED
, _ota_cap{false}
, _ota_error{static_cast<int>(OTAError::None)}
, _ota_img_sha256{"Inv."}
, _ota_url{""}
, _ota_req{false}
, _ask_user_before_executing_ota{false}
, _get_ota_confirmation{nullptr}
#endif /* OTA_ENABLED */
{

}

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

int ArduinoIoTCloudTCP::begin(ConnectionHandler & connection, bool const enable_watchdog, String brokerAddress, uint16_t brokerPort)
{
  _connection = &connection;
  _brokerAddress = brokerAddress;
#ifdef BOARD_HAS_SECRET_KEY
  _brokerPort = _password.length() ? DEFAULT_BROKER_PORT_USER_PASS_AUTH : brokerPort;
#else
  _brokerPort = brokerPort;
#endif
  _time_service.begin(&connection);
  return begin(enable_watchdog, _brokerAddress, _brokerPort);
}

int ArduinoIoTCloudTCP::begin(bool const enable_watchdog, String brokerAddress, uint16_t brokerPort)
{
  _brokerAddress = brokerAddress;
  _brokerPort = brokerPort;

#if OTA_ENABLED
  _ota_img_sha256 = OTA::getImageSHA256();
  DEBUG_VERBOSE("SHA256: HASH(%d) = %s", strlen(_ota_img_sha256.c_str()), _ota_img_sha256.c_str());
#endif /* OTA_ENABLED */

#if defined(BOARD_HAS_SECRET_KEY)
  /* If board is not configured for username and password login */
  if(!_password.length())
  {
#endif
#if defined(BOARD_HAS_SECURE_ELEMENT)
    if (!_selement.begin())
    {
      DEBUG_ERROR("ArduinoIoTCloudTCP::%s could not initialize secure element.", __FUNCTION__);
#if defined(ARDUINO_UNOWIFIR4)
      if (String(WiFi.firmwareVersion()) < String("0.4.1")) {
        DEBUG_ERROR("ArduinoIoTCloudTCP::%s In order to read device certificate, WiFi firmware needs to be >= 0.4.1, current %s", __FUNCTION__, WiFi.firmwareVersion());
      }
#endif
      return 0;
    }
    if (!SElementArduinoCloudDeviceId::read(_selement, getDeviceId(), SElementArduinoCloudSlot::DeviceId))
    {
      DEBUG_ERROR("ArduinoIoTCloudTCP::%s could not read device id.", __FUNCTION__);
      return 0;
    }
  #if !defined(BOARD_HAS_OFFLOADED_ECCX08)
    if (!SElementArduinoCloudCertificate::read(_selement, _cert, SElementArduinoCloudSlot::CompressedCertificate))
    {
      DEBUG_ERROR("ArduinoIoTCloudTCP::%s could not read device certificate.", __FUNCTION__);
      return 0;
    }
    _sslClient.setEccSlot(static_cast<int>(SElementArduinoCloudSlot::Key), _cert.bytes(), _cert.length());
  #endif
#endif
#if defined(BOARD_HAS_SECRET_KEY)
  }
#endif

#if defined(BOARD_HAS_OFFLOADED_ECCX08)

#elif defined(BOARD_HAS_ECCX08)
  _sslClient.setClient(_connection->getClient());
#elif defined(ARDUINO_PORTENTA_C33)
  _sslClient.setClient(_connection->getClient());
  _sslClient.setCACert(AIoTSSCert);
#elif defined(ARDUINO_NICLA_VISION)
  _sslClient.appendCustomCACert(AIoTSSCert);
#elif defined(ARDUINO_EDGE_CONTROL)
  _sslClient.appendCustomCACert(AIoTUPCert);
#elif defined(ARDUINO_UNOR4_WIFI)

#elif defined(ARDUINO_ARCH_ESP32)
  _sslClient.setCACertBundle(x509_crt_bundle);
#elif defined(ARDUINO_ARCH_ESP8266)
  _sslClient.setInsecure();
#endif

  _mqttClient.setClient(_sslClient);
#ifdef BOARD_HAS_SECRET_KEY
  if(_password.length())
  {
    _mqttClient.setUsernamePassword(getDeviceId(), _password);
  }
#endif
  _mqttClient.onMessage(ArduinoIoTCloudTCP::onMessage);
  _mqttClient.setKeepAliveInterval(30 * 1000);
  _mqttClient.setConnectionTimeout(1500);
  _mqttClient.setId(getDeviceId().c_str());

  _deviceTopicOut = getTopic_deviceout();
  _deviceTopicIn  = getTopic_devicein();

  Property* p;
  p = new CloudWrapperString(_lib_version);
  addPropertyToContainer(_device_property_container, *p, "LIB_VERSION", Permission::Read, -1);
#if OTA_ENABLED
  p = new CloudWrapperBool(_ota_cap);
  addPropertyToContainer(_device_property_container, *p, "OTA_CAP", Permission::Read, -1);
  p = new CloudWrapperInt(_ota_error);
  addPropertyToContainer(_device_property_container, *p, "OTA_ERROR", Permission::Read, -1);
  p = new CloudWrapperString(_ota_img_sha256);
  addPropertyToContainer(_device_property_container, *p, "OTA_SHA256", Permission::Read, -1);
  p = new CloudWrapperString(_ota_url);
  addPropertyToContainer(_device_property_container, *p, "OTA_URL", Permission::ReadWrite, -1);
  p = new CloudWrapperBool(_ota_req);
  addPropertyToContainer(_device_property_container, *p, "OTA_REQ", Permission::ReadWrite, -1);
#endif /* OTA_ENABLED */
  p = new CloudWrapperString(_thing_id);
  _thing_id_property = &addPropertyToContainer(_device_property_container, *p, "thing_id", Permission::ReadWrite, -1).writeOnDemand();
  p = new CloudWrapperInt(_tz_offset);
  _tz_offset_property = &addPropertyToContainer(_thing_property_container, *p, "tz_offset", Permission::ReadWrite, -1).writeOnDemand();
  p = new CloudWrapperUnsignedInt(_tz_dst_until);
  _tz_dst_until_property = &addPropertyToContainer(_thing_property_container, *p, "tz_dst_until", Permission::ReadWrite, -1).writeOnDemand();

#if OTA_ENABLED
  _ota_cap = OTA::isCapable();
#endif

#ifdef BOARD_HAS_OFFLOADED_ECCX08
  if (String(WiFi.firmwareVersion()) < String("1.4.4")) {
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s In order to connect to Arduino IoT Cloud, NINA firmware needs to be >= 1.4.4, current %s", __FUNCTION__, WiFi.firmwareVersion());
    return 0;
  }
#endif /* BOARD_HAS_OFFLOADED_ECCX08 */

#if defined(ARDUINO_UNOWIFIR4)
  if (String(WiFi.firmwareVersion()) < String("0.2.0")) {
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s In order to connect to Arduino IoT Cloud, WiFi firmware needs to be >= 0.2.0, current %s", __FUNCTION__, WiFi.firmwareVersion());
  }
#endif

  /* Since we do not control what code the user inserts
   * between ArduinoIoTCloudTCP::begin() and the first
   * call to ArduinoIoTCloudTCP::update() it is wise to
   * set a rather large timeout at first.
   */
#if defined (ARDUINO_ARCH_SAMD) || defined (ARDUINO_ARCH_MBED)
  if (enable_watchdog) {
    /* Initialize watchdog hardware */
    watchdog_enable();
    /* Setup callbacks to feed the watchdog during offloaded network operations (connection/download)*/
    watchdog_enable_network_feed(_connection->getInterface());
  }
#endif

  return 1;
}

void ArduinoIoTCloudTCP::update()
{
  /* Feed the watchdog. If any of the functions called below
   * get stuck than we can at least reset and recover.
   */
#if defined (ARDUINO_ARCH_SAMD) || defined (ARDUINO_ARCH_MBED)
  watchdog_reset();
#endif


  /* Run through the state machine. */
  State next_state = _state;
  switch (_state)
  {
  case State::ConnectPhy:           next_state = handle_ConnectPhy();           break;
  case State::SyncTime:             next_state = handle_SyncTime();             break;
  case State::ConnectMqttBroker:    next_state = handle_ConnectMqttBroker();    break;
  case State::SendDeviceProperties: next_state = handle_SendDeviceProperties(); break;
  case State::SubscribeDeviceTopic: next_state = handle_SubscribeDeviceTopic(); break;
  case State::WaitDeviceConfig:     next_state = handle_WaitDeviceConfig();     break;
  case State::CheckDeviceConfig:    next_state = handle_CheckDeviceConfig();    break;
  case State::SubscribeThingTopics: next_state = handle_SubscribeThingTopics(); break;
  case State::RequestLastValues:    next_state = handle_RequestLastValues();    break;
  case State::Connected:            next_state = handle_Connected();            break;
  case State::Disconnect:           next_state = handle_Disconnect();           break;
  }
  _state = next_state;

#if OTA_ENABLED
  if (_state > State::SubscribeDeviceTopic && _state <= State::Connected) {
    handle_OTARequest();
  }
#endif /* OTA_ENABLED */

  /* This watchdog feed is actually needed only by the RP2040 Connect because its
   * maximum watchdog window is 8389 ms; despite this we feed it for all 
   * supported ARCH to keep code aligned.
   */
#if defined (ARDUINO_ARCH_SAMD) || defined (ARDUINO_ARCH_MBED)
  watchdog_reset();
#endif

  /* Check for new data from the MQTT client. */
  if (_mqttClient.connected())
    _mqttClient.poll();
}

int ArduinoIoTCloudTCP::connected()
{
  return _mqttClient.connected();
}

void ArduinoIoTCloudTCP::printDebugInfo()
{
  DEBUG_INFO("***** Arduino IoT Cloud - configuration info *****");
  DEBUG_INFO("Device ID: %s", getDeviceId().c_str());
  DEBUG_INFO("MQTT Broker: %s:%d", _brokerAddress.c_str(), _brokerPort);
}

/******************************************************************************
 * PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_ConnectPhy()
{
  if (_connection->check() == NetworkConnectionState::CONNECTED)
  {
    bool const is_retry_attempt = (_last_connection_attempt_cnt > 0);
    if (!is_retry_attempt || (is_retry_attempt && (millis() > _next_connection_attempt_tick)))
      return State::SyncTime;
  }

  return State::ConnectPhy;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_SyncTime()
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
  unsigned long const internal_posix_time = _time_service.getTime();
#pragma GCC diagnostic pop
  DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s internal clock configured to posix timestamp %d", __FUNCTION__, internal_posix_time);
  return State::ConnectMqttBroker;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_ConnectMqttBroker()
{
  if (_mqttClient.connect(_brokerAddress.c_str(), _brokerPort))
  {
    _last_connection_attempt_cnt = 0;
    return State::SendDeviceProperties;
  }

  /* Can't connect to the broker. Wait: 2s -> 4s -> 8s -> 16s -> 32s -> 32s ... */
  _last_connection_attempt_cnt++;
  unsigned long reconnection_retry_delay = (1 << _last_connection_attempt_cnt) * AIOT_CONFIG_RECONNECTION_RETRY_DELAY_ms;
  reconnection_retry_delay = min(reconnection_retry_delay, static_cast<unsigned long>(AIOT_CONFIG_MAX_RECONNECTION_RETRY_DELAY_ms));
  _next_connection_attempt_tick = millis() + reconnection_retry_delay;

  DEBUG_ERROR("ArduinoIoTCloudTCP::%s could not connect to %s:%d", __FUNCTION__, _brokerAddress.c_str(), _brokerPort);
  DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s %d next connection attempt in %d ms", __FUNCTION__, _last_connection_attempt_cnt, reconnection_retry_delay);
  /* Go back to ConnectPhy and retry to get time from network (invalid time for SSL handshake?)*/
  return State::ConnectPhy;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_SendDeviceProperties()
{
  if (!_mqttClient.connected())
  {
    return State::Disconnect;
  }

  DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s announce device to the Cloud %d", __FUNCTION__, _time_service.getTime());
  /* TODO check if write fails */
  sendDevicePropertiesToCloud();
  return State::WaitDeviceConfig;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_WaitDeviceConfig()
{
  if (!_mqttClient.connected())
  {
    return State::Disconnect;
  }

  bool const is_retry_attempt = (_last_device_subscribe_cnt > 0);
  if (is_retry_attempt && (millis() > _next_device_subscribe_attempt_tick))
  {
    /* Configuration not received or device not attached to a valid thing. Try to resubscribe */
    if (_mqttClient.unsubscribe(_deviceTopicIn))
    {
      DEBUG_ERROR("ArduinoIoTCloudTCP::%s device waiting for valid thing_id %d", __FUNCTION__, _time_service.getTime());
    }
  }

  if (!is_retry_attempt || (is_retry_attempt && (millis() > _next_device_subscribe_attempt_tick)))
    return State::SubscribeDeviceTopic;

  return State::WaitDeviceConfig;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_SubscribeDeviceTopic()
{
  if (!_mqttClient.connected())
  {
    return State::Disconnect;
  }

  DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s request device configuration %d", __FUNCTION__, _time_service.getTime());

  if (!_mqttClient.subscribe(_deviceTopicIn))
  {
    /* If device_id is wrong the board can't connect to the broker so this condition
    * should never happen.
    */
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s could not subscribe to %s", __FUNCTION__, _deviceTopicIn.c_str());
  }

  /* Max retry than disconnect */
  if (_last_device_subscribe_cnt > AIOT_CONFIG_LASTVALUES_SYNC_MAX_RETRY_CNT)
  {
    _last_device_subscribe_cnt = 0;
    return State::Disconnect;
  }

  /* No device configuration received. Wait: 4s -> 8s -> 16s -> 32s -> 32s ...*/
  _last_device_subscribe_cnt++;
  unsigned long subscribe_retry_delay = (1 << _last_device_subscribe_cnt) * AIOT_CONFIG_DEVICE_TOPIC_SUBSCRIBE_RETRY_DELAY_ms;
  subscribe_retry_delay = min(subscribe_retry_delay, static_cast<unsigned long>(AIOT_CONFIG_MAX_DEVICE_TOPIC_SUBSCRIBE_RETRY_DELAY_ms));
  _next_device_subscribe_attempt_tick = millis() + subscribe_retry_delay;
  DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s %d next configuration request in %d ms", __FUNCTION__, _last_device_subscribe_cnt, subscribe_retry_delay);

  return State::WaitDeviceConfig;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_CheckDeviceConfig()
{
  if (!_mqttClient.connected())
  {
    return State::Disconnect;
  }

  updateThingTopics();

  if (_thing_id.length() == 0)
  {
    /* Device configuration received, but invalid thing_id. Do not increase counter, but recompute delay.
    * Device not attached. Wait: 40s -> 80s -> 160s -> 320s -> 640s -> 1280s -> 1280s ...
    */
    unsigned long attach_retry_delay = (1 << _last_device_subscribe_cnt) * AIOT_CONFIG_DEVICE_TOPIC_ATTACH_RETRY_DELAY_ms;
    attach_retry_delay = min(attach_retry_delay, static_cast<unsigned long>(AIOT_CONFIG_MAX_DEVICE_TOPIC_ATTACH_RETRY_DELAY_ms));
    _next_device_subscribe_attempt_tick = millis() + attach_retry_delay;

    DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s device not attached, next configuration request in %d ms", __FUNCTION__, attach_retry_delay);
    return State::WaitDeviceConfig;
  }

  DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s device attached to a new valid thing_id %s %d", __FUNCTION__, getThingId().c_str(), _time_service.getTime());

  /* Received valid thing_id reset counters and go on */
  _last_device_subscribe_cnt = 0;

  return State::SubscribeThingTopics;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_SubscribeThingTopics()
{
  if (!_mqttClient.connected())
  {
    return State::Disconnect;
  }

  if (_thing_id_property->isDifferentFromCloud())
  {
    return State::Disconnect;
  }

  bool const is_retry_attempt = (_last_thing_subscribe_attempt_cnt > 0);
  if (is_retry_attempt && (millis() < _next_thing_subscribe_attempt_tick))
    return State::SubscribeThingTopics;

  if (_last_thing_subscribe_attempt_cnt > AIOT_CONFIG_THING_TOPICS_SUBSCRIBE_MAX_RETRY_CNT)
  {
    _last_thing_subscribe_attempt_cnt = 0;
    return State::Disconnect;
  }

  _next_thing_subscribe_attempt_tick = millis() + AIOT_CONFIG_THING_TOPICS_SUBSCRIBE_RETRY_DELAY_ms;
  _last_thing_subscribe_attempt_cnt++;

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
  execCloudEventCallback(ArduinoIoTCloudEvent::CONNECT);

  return State::RequestLastValues;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_RequestLastValues()
{
  if (!_mqttClient.connected())
  {
    return State::Disconnect;
  }

  if (_thing_id_property->isDifferentFromCloud())
  {
    return State::Disconnect;
  }

  /* Check whether or not we need to send a new request. */
  bool const is_retry_attempt = (_last_sync_attempt_cnt > 0);
  if (is_retry_attempt && (millis() < _next_sync_attempt_tick))
    return State::RequestLastValues;

  if (_last_sync_attempt_cnt > AIOT_CONFIG_LASTVALUES_SYNC_MAX_RETRY_CNT)
  {
    /* Track the number of times a get-last-values request was sent to the cloud.
     * If no data is received within a certain number of retry-requests it's a better
     * strategy to disconnect and re-establish connection from the ground up.
     */
    _last_sync_attempt_cnt = 0;
    return State::Disconnect;
  }

  DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s [%d] last values requested", __FUNCTION__, _time_service.getTime());
  requestLastValue();
  _next_sync_attempt_tick = millis() + AIOT_CONFIG_TIMEOUT_FOR_LASTVALUES_SYNC_ms;
  _last_sync_attempt_cnt++;

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
    if (_thing_id_property->isDifferentFromCloud())
    {
      return State::Disconnect;
    }

    /* Check if a primitive property wrapper is locally changed.
    * This function requires an existing time service which in
    * turn requires an established connection. Not having that
    * leads to a wrong time set in the time service which inhibits
    * the connection from being established due to a wrong data
    * in the reconstructed certificate.
    */
    updateTimestampOnLocallyChangedProperties(_thing_property_container);

    /* Retransmit data in case there was a lost transaction due
    * to phy layer or MQTT connectivity loss.
    */
    if (_mqtt_data_request_retransmit && (_mqtt_data_len > 0)) {
      write(_dataTopicOut, _mqtt_data_buf, _mqtt_data_len);
      _mqtt_data_request_retransmit = false;
    }

    /* Configure Time service with timezone data:
    * _tz_offset [offset + dst]
    * _tz_dst_until [posix timestamp until _tz_offset is valid]
    */
    if (_tz_offset_property->isDifferentFromCloud() || _tz_dst_until_property->isDifferentFromCloud()) {
      _tz_offset_property->fromCloudToLocal();
      _tz_dst_until_property->fromCloudToLocal();
      _time_service.setTimeZoneData(_tz_offset, _tz_dst_until);
    }

    /* Check if any properties need encoding and send them to
    * the cloud if necessary.
    */
    sendThingPropertiesToCloud();

    unsigned long const internal_posix_time = _time_service.getTime();
    if (internal_posix_time < _tz_dst_until) {
      return State::Connected;
    } else {
      return State::RequestLastValues;
    }
  }
}

#if OTA_ENABLED
void ArduinoIoTCloudTCP::handle_OTARequest() {
  /* Request a OTA download if the hidden property
  * OTA request has been set.
  */

  if (_ota_req)
  {
    bool const ota_execution_allowed_by_user = (_get_ota_confirmation != nullptr && _get_ota_confirmation());
    bool const perform_ota_now = ota_execution_allowed_by_user || !_ask_user_before_executing_ota;
    if (perform_ota_now) {
      /* Clear the error flag. */
      _ota_error = static_cast<int>(OTAError::None);
      /* Clear the request flag. */
      _ota_req = false;
      /* Transmit the cleared request flags to the cloud. */
      sendDevicePropertyToCloud("OTA_REQ");
      /* Call member function to handle OTA request. */
      _ota_error = OTA::onRequest(_ota_url, _connection->getInterface());
      /* If something fails send the OTA error to the cloud */
      sendDevicePropertyToCloud("OTA_ERROR");
    }
  }

  /* Check if we have received the OTA_URL property and provide
  * echo to the cloud.
  */
  sendDevicePropertyToCloud("OTA_URL");
}
#endif /* OTA_ENABLED */

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_Disconnect()
{
  if (!_mqttClient.connected()) {
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s MQTT client connection lost", __FUNCTION__);
  } else {
    _mqttClient.unsubscribe(_shadowTopicIn);
    _mqttClient.unsubscribe(_dataTopicIn);
    _mqttClient.stop();
  }
  DEBUG_INFO("Disconnected from Arduino IoT Cloud");
  execCloudEventCallback(ArduinoIoTCloudEvent::DISCONNECT);
  return State::ConnectPhy;
}

void ArduinoIoTCloudTCP::onMessage(int length)
{
  ArduinoCloud.handleMessage(length);
}

void ArduinoIoTCloudTCP::handleMessage(int length)
{
  String topic = _mqttClient.messageTopic();

  byte bytes[length];

  for (int i = 0; i < length; i++) {
    bytes[i] = _mqttClient.read();
  }

  /* Topic for OTA properties and device configuration */
  if (_deviceTopicIn == topic) {
    CBORDecoder::decode(_device_property_container, (uint8_t*)bytes, length);
    _state = State::CheckDeviceConfig;
  }

  /* Topic for user input data */
  if (_dataTopicIn == topic) {
    CBORDecoder::decode(_thing_property_container, (uint8_t*)bytes, length);
  }

  /* Topic for sync Thing last values on connect */
  if ((_shadowTopicIn == topic) && (_state == State::RequestLastValues))
  {
    DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s [%d] last values received", __FUNCTION__, millis());
    CBORDecoder::decode(_thing_property_container, (uint8_t*)bytes, length, true);
    _time_service.setTimeZoneData(_tz_offset, _tz_dst_until);
    execCloudEventCallback(ArduinoIoTCloudEvent::SYNC);
    _last_sync_attempt_cnt = 0;
    _state = State::Connected;
  }
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
  sendPropertyContainerToCloud(_dataTopicOut, _thing_property_container, _last_checked_property_index);
}

void ArduinoIoTCloudTCP::sendDevicePropertiesToCloud()
{
  PropertyContainer ro_device_property_container;
  unsigned int last_device_property_index = 0;

  std::list<String> ro_device_property_list {"LIB_VERSION", "OTA_CAP", "OTA_ERROR", "OTA_SHA256"};
  std::for_each(ro_device_property_list.begin(),
                ro_device_property_list.end(),
                [this, &ro_device_property_container ] (String const & name)
                {
                  Property* p = getProperty(this->_device_property_container, name);
                  if(p != nullptr)
                    addPropertyToContainer(ro_device_property_container, *p, p->name(), p->isWriteableByCloud() ? Permission::ReadWrite : Permission::Read);
                }
                );

  sendPropertyContainerToCloud(_deviceTopicOut, ro_device_property_container, last_device_property_index);
}

#if OTA_ENABLED
void ArduinoIoTCloudTCP::sendDevicePropertyToCloud(String const name)
{
  PropertyContainer temp_device_property_container;
  unsigned int last_device_property_index = 0;

  Property* p = getProperty(this->_device_property_container, name);
  if(p != nullptr)
  {
    addPropertyToContainer(temp_device_property_container, *p, p->name(), p->isWriteableByCloud() ? Permission::ReadWrite : Permission::Read);
    sendPropertyContainerToCloud(_deviceTopicOut, temp_device_property_container, last_device_property_index);
  }
}
#endif

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
  _thing_id_property->fromCloudToLocal();

  _shadowTopicOut = getTopic_shadowout();
  _shadowTopicIn  = getTopic_shadowin();
  _dataTopicOut   = getTopic_dataout();
  _dataTopicIn    = getTopic_datain();
}

/******************************************************************************
 * EXTERN DEFINITION
 ******************************************************************************/

ArduinoIoTCloudTCP ArduinoCloud;

#endif
