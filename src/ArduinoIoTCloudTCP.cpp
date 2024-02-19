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

#ifdef BOARD_HAS_ECCX08
  #include "tls/BearSSLTrustAnchors.h"
  #include "tls/utility/CryptoUtil.h"
#endif

#ifdef BOARD_HAS_SE050
  #include "tls/AIoTCSSCert.h"
  #include "tls/utility/CryptoUtil.h"
#endif

#ifdef BOARD_HAS_OFFLOADED_ECCX08
  #include <ArduinoECCX08.h>
  #include "tls/utility/CryptoUtil.h"
#endif

#ifdef BOARD_HAS_SECRET_KEY
  #include "tls/AIoTCUPCert.h"
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

void updateTimezoneInfo()
{
  ArduinoCloud.updateInternalTimezoneInfo();
}

void setThingIdOutdated()
{
  ArduinoCloud.setThingIdOutdatedFlag();
}

void executeEventCallback(ArduinoIoTCloudEvent const event)
{
  ArduinoCloud.execCloudEventCallback(event);
}

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

ArduinoIoTCloudTCP::ArduinoIoTCloudTCP()
: _state{State::ConnectPhy}
, _next_connection_attempt_tick{0}
, _last_connection_attempt_cnt{0}
, _next_device_subscribe_attempt_tick{0}
, _last_device_subscribe_cnt{0}
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
, _arduinoCloudThing()
, _deviceTopicOut("")
, _deviceTopicIn("")
, _shadowTopicIn("")
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
  _brokerPort = brokerPort;
  _time_service.begin(&connection);
  _arduinoCloudThing.begin(&_mqttClient, &_time_service, executeEventCallback);
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

#if defined(BOARD_HAS_ECCX08) || defined(BOARD_HAS_OFFLOADED_ECCX08) || defined(BOARD_HAS_SE050)
  if (!_crypto.begin())
  {
    DEBUG_ERROR("_crypto.begin() failed.");
    return 0;
  }
  if (!_crypto.readDeviceId(getDeviceId(), CryptoSlot::DeviceId))
  {
    DEBUG_ERROR("_crypto.readDeviceId(...) failed.");
    return 0;
  }
#endif

#if defined(BOARD_HAS_ECCX08) || defined(BOARD_HAS_SE050)
  if (!_crypto.readCert(_cert, CryptoSlot::CompressedCertificate))
  {
    DEBUG_ERROR("Cryptography certificate reconstruction failure.");
    return 0;
  }
  _sslClient.setEccSlot(static_cast<int>(CryptoSlot::Key), _cert.bytes(), _cert.length());
#endif

#if defined(BOARD_HAS_ECCX08)
  _sslClient.setClient(_connection->getClient());
#elif defined(ARDUINO_PORTENTA_C33)
  _sslClient.setClient(_connection->getClient());
  _sslClient.setCACert(AIoTSSCert);
#elif defined(BOARD_HAS_SE050)
  _sslClient.appendCustomCACert(AIoTSSCert);
#elif defined(BOARD_ESP)
  #if defined(ARDUINO_ARCH_ESP8266)
  _sslClient.setInsecure();
  #else
  _sslClient.setCACertBundle(x509_crt_bundle);
  #endif
#elif defined(ARDUINO_EDGE_CONTROL)
  _sslClient.appendCustomCACert(AIoTUPCert);
#endif

  _mqttClient.setClient(_sslClient);
#ifdef BOARD_HAS_SECRET_KEY
  _mqttClient.setUsernamePassword(getDeviceId(), _password);
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
  p = new CloudWrapperString(_arduinoCloudThing.getThingId());
  addPropertyToContainer(_device_property_container, *p, "thing_id", Permission::ReadWrite, -1).onUpdate(setThingIdOutdated);
  addPropertyReal(_tz_offset, "tz_offset", Permission::ReadWrite).onSync(CLOUD_WINS).onUpdate(updateTimezoneInfo);
  addPropertyReal(_tz_dst_until, "tz_dst_until", Permission::ReadWrite).onSync(CLOUD_WINS).onUpdate(updateTimezoneInfo);

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

Property& ArduinoIoTCloudTCP::addInternalPropertyReal(Property& property, String name, int tag, Permission const permission)
{
  return _arduinoCloudThing.addPropertyReal(property, name, tag, permission);
}

void ArduinoIoTCloudTCP::addInternalPropertyReal(Property& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  Permission permission = Permission::ReadWrite;
  if (permission_type == READ) {
    permission = Permission::Read;
  } else if (permission_type == WRITE) {
    permission = Permission::Write;
  } else {
    permission = Permission::ReadWrite;
  }
  _arduinoCloudThing.addPropertyReal(property, name, tag, permission).publishOnChange(minDelta, Property::DEFAULT_MIN_TIME_BETWEEN_UPDATES_MILLIS).onUpdate(fn).onSync(synFn);
}

void ArduinoIoTCloudTCP::push()
{
  _arduinoCloudThing.push();
}

bool ArduinoIoTCloudTCP::setTimestamp(String const & prop_name, unsigned long const timestamp)
{
  return _arduinoCloudThing.setTimestamp(prop_name, timestamp);
}

bool ArduinoIoTCloudTCP::deviceNotAttached()
{
  return _arduinoCloudThing.getThingId().length() == 0;
}

void ArduinoIoTCloudTCP::setThingId(String const thing_id)
{
  _arduinoCloudThing.setThingId(thing_id);
}

String& ArduinoIoTCloudTCP::getThingId()
{
  return _arduinoCloudThing.getThingId();
}

void ArduinoIoTCloudTCP::setThingIdOutdatedFlag()
{
  _arduinoCloudThing.setThingIdOutdatedFlag();
}

void ArduinoIoTCloudTCP::clrThingIdOutdatedFlag()
{
  _arduinoCloudThing.clrThingIdOutdatedFlag();
}

bool ArduinoIoTCloudTCP::getThingIdOutdatedFlag()
{
  return _arduinoCloudThing.getThingIdOutdatedFlag();
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
  {
    _mqttClient.poll();
  }
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
    _arduinoCloudThing.clrLastValueReceived();
    _last_connection_attempt_cnt = 0;
    return State::SendDeviceProperties;
  }

  _last_connection_attempt_cnt++;
  unsigned long reconnection_retry_delay = (1 << _last_connection_attempt_cnt) * AIOT_CONFIG_RECONNECTION_RETRY_DELAY_ms;
  reconnection_retry_delay = min(reconnection_retry_delay, static_cast<unsigned long>(AIOT_CONFIG_MAX_RECONNECTION_RETRY_DELAY_ms));
  _next_connection_attempt_tick = millis() + reconnection_retry_delay;

  DEBUG_ERROR("ArduinoIoTCloudTCP::%s could not connect to %s:%d", __FUNCTION__, _brokerAddress.c_str(), _brokerPort);
  DEBUG_ERROR("ArduinoIoTCloudTCP::%s %d connection attempt at tick time %d", __FUNCTION__, _last_connection_attempt_cnt, _next_connection_attempt_tick);
  return State::ConnectPhy;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_SendDeviceProperties()
{
  if (!_mqttClient.connected())
  {
    return State::Disconnect;
  }

  sendDevicePropertiesToCloud();
  return State::SubscribeDeviceTopic;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_SubscribeDeviceTopic()
{
  if (!_mqttClient.connected())
  {
    return State::Disconnect;
  }

  if (!_mqttClient.subscribe(_deviceTopicIn))
  {
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s could not subscribe to %s", __FUNCTION__, _deviceTopicIn.c_str());
    return State::SubscribeDeviceTopic;
  }

  if (_last_device_subscribe_cnt > AIOT_CONFIG_LASTVALUES_SYNC_MAX_RETRY_CNT)
  {
    _last_device_subscribe_cnt = 0;
    _next_device_subscribe_attempt_tick = 0;
    _mqttClient.stop();
    execCloudEventCallback(ArduinoIoTCloudEvent::DISCONNECT);
    return State::ConnectPhy;
  }

  /* No device configuration reply. Wait: 5s -> 10s -> 20s -> 30s */
  unsigned long subscribe_retry_delay = (1 << _last_device_subscribe_cnt) * AIOT_CONFIG_DEVICE_TOPIC_SUBSCRIBE_RETRY_DELAY_ms;
  subscribe_retry_delay = min(subscribe_retry_delay, static_cast<unsigned long>(AIOT_CONFIG_MAX_DEVICE_TOPIC_SUBSCRIBE_RETRY_DELAY_ms));
  _next_device_subscribe_attempt_tick = millis() + subscribe_retry_delay;
  _last_device_subscribe_cnt++;

  return State::Connected;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_Connected()
{
  if (!_mqttClient.connected())
  {
    /* The last message was definitely lost, trigger a retransmit. */
    //TODO set the variable in the thing class
    _mqtt_data_request_retransmit = true;
    return State::Disconnect;
  }
  /* We are connected so let's to our stuff here. */
  else
  {
    if (getThingIdOutdatedFlag())
    {
      _arduinoCloudThing.setThingIdOutdatedFlag();
    }

    /* Retransmit data in case there was a lost transaction due
    * to phy layer or MQTT connectivity loss.
    */
    if(_mqtt_data_request_retransmit) {
      _arduinoCloudThing.setMqttDataRequestRetransmitFlag();
    }

#if OTA_ENABLED
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

#endif /* OTA_ENABLED */

    // update the thing state machine
    _arduinoCloudThing.update();

    return State::Connected;
  }
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_Disconnect()
{
  DEBUG_ERROR("ArduinoIoTCloudTCP::%s MQTT client connection lost", __FUNCTION__);
  _mqttClient.stop();
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
    _last_device_subscribe_cnt = 0;
    _next_device_subscribe_attempt_tick = 0;
  }

  /* Topics for thing input data */
  updateThingTopics();
  if (_dataTopicIn == topic || _shadowTopicIn == topic) {
    updateTimezoneInfo();
    _arduinoCloudThing.handleMessage(topic, (uint8_t*)bytes, length);
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
  _shadowTopicIn  = getTopic_shadowin();
  _dataTopicIn    = getTopic_datain();

  clrThingIdOutdatedFlag();
}

/******************************************************************************
 * EXTERN DEFINITION
 ******************************************************************************/

ArduinoIoTCloudTCP ArduinoCloud;

#endif
