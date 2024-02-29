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
#include <typeinfo>

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
, _connection_attempt(0,0)
, _message_stream(std::bind(&ArduinoIoTCloudTCP::sendMessage, this, std::placeholders::_1))
, _thing(&_message_stream)
, _device(&_message_stream)
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
, _messageTopicOut("")
, _messageTopicIn("")
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

  /* Setup TimeService */
  _time_service.begin(&connection);

  /* Setup retry timers */
  _connection_attempt.begin(AIOT_CONFIG_RECONNECTION_RETRY_DELAY_ms, AIOT_CONFIG_MAX_RECONNECTION_RETRY_DELAY_ms);
  return begin(enable_watchdog, _brokerAddress, _brokerPort);
}

int ArduinoIoTCloudTCP::begin(bool const enable_watchdog, String brokerAddress, uint16_t brokerPort)
{
  _brokerAddress = brokerAddress;
  _brokerPort = brokerPort;

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
  _messageTopicIn = getTopic_messagein();
  _messageTopicOut = getTopic_messageout();

  _thing.begin();
  _device.begin();

#if  OTA_ENABLED
  Property* p;
  p = new CloudWrapperBool(_ota_cap);
  addPropertyToContainer(_device.getPropertyContainer(), *p, "OTA_CAP", Permission::Read, -1);
  p = new CloudWrapperInt(_ota_error);
  addPropertyToContainer(_device.getPropertyContainer(), *p, "OTA_ERROR", Permission::Read, -1);
  p = new CloudWrapperString(_ota_img_sha256);
  addPropertyToContainer(_device.getPropertyContainer(), *p, "OTA_SHA256", Permission::Read, -1);
  p = new CloudWrapperString(_ota_url);
  addPropertyToContainer(_device.getPropertyContainer(), *p, "OTA_URL", Permission::ReadWrite, -1);
  p = new CloudWrapperBool(_ota_req);
  addPropertyToContainer(_device.getPropertyContainer(), *p, "OTA_REQ", Permission::ReadWrite, -1);

  _ota_cap = OTA::isCapable();

  _ota_img_sha256 = OTA::getImageSHA256();
  DEBUG_VERBOSE("SHA256: HASH(%d) = %s", strlen(_ota_img_sha256.c_str()), _ota_img_sha256.c_str());
#endif // OTA_ENABLED

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
  case State::Connected:            next_state = handle_Connected();            break;
  case State::Disconnect:           next_state = handle_Disconnect();           break;
  }
  _state = next_state;

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
    if (!_connection_attempt.isRetry() || (_connection_attempt.isRetry() && _connection_attempt.isExpired()))
      return State::SyncTime;
  }

  return State::ConnectPhy;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_SyncTime()
{
  if (TimeServiceClass::isTimeValid(getTime()))
  {
    DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s internal clock configured to posix timestamp %d", __FUNCTION__, getTime());
    return State::ConnectMqttBroker;
  }

  return State::ConnectPhy;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_ConnectMqttBroker()
{
  if (_mqttClient.connect(_brokerAddress.c_str(), _brokerPort))
  {
    /* Subscribe to message topic to receive commands */
    _mqttClient.subscribe(_messageTopicIn);

    /* Temoporarly subscribe to device topic to receive OTA properties */
    _mqttClient.subscribe(_deviceTopicIn);

    /* Reconfigure timers for next state */
    _connection_attempt.begin(AIOT_CONFIG_DEVICE_TOPIC_SUBSCRIBE_RETRY_DELAY_ms, AIOT_CONFIG_MAX_DEVICE_TOPIC_SUBSCRIBE_RETRY_DELAY_ms);

    DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s connected to %s:%d", __FUNCTION__, _brokerAddress.c_str(), _brokerPort);
    return State::Connected;
  }

  /* Can't connect to the broker. Wait: 2s -> 4s -> 8s -> 16s -> 32s -> 32s ... */
  _connection_attempt.retry();

  DEBUG_ERROR("ArduinoIoTCloudTCP::%s could not connect to %s:%d", __FUNCTION__, _brokerAddress.c_str(), _brokerPort);
  DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s %d next connection attempt in %d ms", __FUNCTION__, _connection_attempt.getRetryCount(), _connection_attempt.getWaitTime());
  /* Go back to ConnectPhy and retry to get time from network (invalid time for SSL handshake?)*/
  return State::ConnectPhy;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_Connected()
{
  if (!_mqttClient.connected() || !_thing.connected() || !_device.connected())
  {
    Serial.println("State::Disconnect");
    return State::Disconnect;
  }

  /* Retransmit data in case there was a lost transaction due
   * to phy layer or MQTT connectivity loss.
   */
  if (_mqtt_data_request_retransmit && (_mqtt_data_len > 0)) {
    write(_dataTopicOut, _mqtt_data_buf, _mqtt_data_len);
    _mqtt_data_request_retransmit = false;
  }

  /* Call CloudDevice process to get configuration */
  _device.update();

  if (_device.isAttached()) {
    /* Call CloudThing process to synchronize properties */
    _thing.update();
  }

#if OTA_ENABLED
  if (_device.connected()) {
    handle_OTARequest();
  }
#endif /* OTA_ENABLED */

  return State::Connected;
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
    /* No need to manually unsubscribe because we are using clean sessions */
    _mqttClient.stop();
  }

  Message message = { ResetCmdId };
  _thing.handleMessage(&message);
  _device.handleMessage(&message);

  DEBUG_INFO("Disconnected from Arduino IoT Cloud");
  execCloudEventCallback(ArduinoIoTCloudEvent::DISCONNECT);

  /* Setup timer for broker connection and restart */
  _connection_attempt.begin(AIOT_CONFIG_RECONNECTION_RETRY_DELAY_ms, AIOT_CONFIG_MAX_RECONNECTION_RETRY_DELAY_ms);
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
    CBORDecoder::decode(_device.getPropertyContainer(), (uint8_t*)bytes, length);
  }

  /* Topic for user input data */
  if (_dataTopicIn == topic) {
    CBORDecoder::decode(_thing.getPropertyContainer(), (uint8_t*)bytes, length);
  }

  /* Topic for device commands */
  if (_messageTopicIn == topic) {
    CommandDown command;
    DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s [%d] received %d bytes", __FUNCTION__, millis(), length);
    CBORMessageDecoder decoder;

    size_t buffer_length = length;
    if (decoder.decode((Message*)&command, bytes, buffer_length) != Decoder::Status::Error) {
      DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s [%d] received command id %d", __FUNCTION__, millis(), command.c.id);
      switch (command.c.id)
      {
        case CommandId::ThingUpdateCmdId:
        {
          DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s [%d] device configuration received", __FUNCTION__, millis());
          if ( _thing_id != String(command.thingUpdateCmd.params.thing_id)) {
            _thing_id = String(command.thingUpdateCmd.params.thing_id);
            Message message;
            /* If we are attached we need first to detach */
            if (_device.isAttached()) {
              detachThing();
              message = { DeviceDetachedCmdId };
            }
            /* If received thing id is valid attach to the new thing */
            if (_thing_id.length()) {
              attachThing();
              message = { DeviceAttachedCmdId };
            } else {
              /* Send message to device state machine to inform we have received a null thing-id */
              _thing_id = "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx";
              message = { DeviceRegisteredCmdId };
            }
            _device.handleMessage(&message);
          }
        }
        break;

        case CommandId::LastValuesUpdateCmdId:
        {
          DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s [%d] last values received", __FUNCTION__, millis());
          CBORDecoder::decode(_thing.getPropertyContainer(),
            (uint8_t*)command.lastValuesUpdateCmd.params.last_values,
            command.lastValuesUpdateCmd.params.length, true);
          _thing.handleMessage((Message*)&command);
          execCloudEventCallback(ArduinoIoTCloudEvent::SYNC);

          /*
           * NOTE: in this current version properties are not properly integrated with the new paradigm of
           * modeling the messages with C structs. The current CBOR library allocates an array in the heap
           * thus we need to delete it after decoding it with the old CBORDecoder
           */
          free(command.lastValuesUpdateCmd.params.last_values);
        }
        break;

        default:
        break;
      }
    }
  }
}

void ArduinoIoTCloudTCP::sendMessage(Message * msg)
{
  uint8_t data[MQTT_TRANSMIT_BUFFER_SIZE];
  size_t bytes_encoded = sizeof(data);
  CBORMessageEncoder encoder;

  switch (msg->id) {
    case PropertiesUpdateCmdId:
      return sendPropertyContainerToCloud(_dataTopicOut,
                                          _thing.getPropertyContainer(),
                                          _thing.getPropertyContainerIndex());
      break;

#if OTA_ENABLED
    case DeviceBeginCmdId:
      sendDevicePropertyToCloud("OTA_CAP");
      sendDevicePropertyToCloud("OTA_ERROR");
      sendDevicePropertyToCloud("OTA_SHA256");
      break;
#endif

    default:
      break;
  }

  if (encoder.encode(msg, data, bytes_encoded) == Encoder::Status::Complete &&
      bytes_encoded > 0) {
    write(_messageTopicOut, data, bytes_encoded);
  } else {
    DEBUG_ERROR("error encoding %d", msg->id);
  }
}

void ArduinoIoTCloudTCP::sendPropertyContainerToCloud(String const topic, PropertyContainer & property_container, unsigned int & current_property_index)
{
  int bytes_encoded = 0;
  uint8_t data[MQTT_TRANSMIT_BUFFER_SIZE];

  if (CBOREncoder::encode(property_container, data, sizeof(data), bytes_encoded, current_property_index, false) == CborNoError)
  {
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
}

#if OTA_ENABLED
void ArduinoIoTCloudTCP::sendDevicePropertyToCloud(String const name)
{
  PropertyContainer temp_device_property_container;
  unsigned int last_device_property_index = 0;

  Property* p = getProperty(this->_device.getPropertyContainer(), name);
  if(p != nullptr)
  {
    addPropertyToContainer(temp_device_property_container, *p, p->name(), p->isWriteableByCloud() ? Permission::ReadWrite : Permission::Read);
    sendPropertyContainerToCloud(_deviceTopicOut, temp_device_property_container, last_device_property_index);
  }
}
#endif

void ArduinoIoTCloudTCP::attachThing()
{

  _dataTopicIn    = getTopic_datain();
  _dataTopicOut   = getTopic_dataout();
  if (!_mqttClient.subscribe(_dataTopicIn)) {
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s could not subscribe to %s", __FUNCTION__, _dataTopicIn.c_str());
    DEBUG_ERROR("Check your thing configuration, and press the reset button on your board.");
    return;
  }

  DEBUG_INFO("Connected to Arduino IoT Cloud");
  DEBUG_INFO("Thing ID: %s", getThingId().c_str());
  execCloudEventCallback(ArduinoIoTCloudEvent::CONNECT);
}

void ArduinoIoTCloudTCP::detachThing()
{
  if (!_mqttClient.unsubscribe(_dataTopicIn)) {
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s could not unsubscribe from %s", __FUNCTION__, _dataTopicIn.c_str());
    return;
  }

  DEBUG_INFO("Disconnected from Arduino IoT Cloud");
  execCloudEventCallback(ArduinoIoTCloudEvent::DISCONNECT);
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

/******************************************************************************
 * EXTERN DEFINITION
 ******************************************************************************/

ArduinoIoTCloudTCP ArduinoCloud;

#endif
