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
#include "utility/time/TimeService.h"
#ifdef BOARD_HAS_ECCX08
  #include "tls/BearSSLTrustAnchors.h"
  #include "tls/utility/CryptoUtil.h"
#endif

#include "utility/ota/FlashSHA256.h"
#include "utility/ota/OTAStorage_SNU.h"
#include "utility/ota/OTAStorage_SFU.h"
#include "utility/ota/OTAStorage_SSU.h"

#include "cbor/CBOREncoder.h"

/******************************************************************************
   GLOBAL VARIABLES
 ******************************************************************************/

TimeService time_service;

#if   OTA_STORAGE_SSU
  static OTAStorage_SSU ota_storage_ssu;
#elif OTA_STORAGE_SFU
  static OTAStorage_SFU ota_storage_sfu;
#elif OTA_STORAGE_SNU
  static OTAStorage_SNU ota_storage_snu;
#endif

/******************************************************************************
   GLOBAL CONSTANTS
 ******************************************************************************/

static const int TIMEOUT_FOR_LASTVALUES_SYNC = 10000;

const static int CONNECT_SUCCESS							               = 1;
const static int CONNECT_FAILURE							               = 0;
const static int CONNECT_FAILURE_SUBSCRIBE					         = -1;

/******************************************************************************
   LOCAL MODULE FUNCTIONS
 ******************************************************************************/

extern "C" unsigned long getTime()
{
  return time_service.getTime();
}

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

ArduinoIoTCloudTCP::ArduinoIoTCloudTCP()
: _state{State::ConnectMqttBroker}
, _lastSyncRequestTickTime{0}
, _mqtt_data_buf{0}
, _mqtt_data_len{0}
, _mqtt_data_request_retransmit{false}
#ifdef BOARD_HAS_ECCX08
, _sslClient(nullptr, ArduinoIoTCloudTrustAnchor, ArduinoIoTCloudTrustAnchor_NUM, getTime)
#endif
  #ifdef BOARD_ESP
, _password("")
  #endif
, _mqttClient{nullptr}
, _stdinTopic("")
, _stdoutTopic("")
, _shadowTopicOut("")
, _shadowTopicIn("")
, _dataTopicOut("")
, _dataTopicIn("")
, _ota_topic_in{""}
#if OTA_ENABLED
, _ota_error{static_cast<int>(OTAError::None)}
, _ota_img_sha256{"Inv."}
#endif /* OTA_ENABLED */
{

}

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

int ArduinoIoTCloudTCP::begin(ConnectionHandler & connection, String brokerAddress, uint16_t brokerPort)
{
  _connection = &connection;
  _brokerAddress = brokerAddress;
  _brokerPort = brokerPort;
  time_service.begin(&connection);
  return begin(_brokerAddress, _brokerPort);
}

int ArduinoIoTCloudTCP::begin(String brokerAddress, uint16_t brokerPort)
{

  _brokerAddress = brokerAddress;
  _brokerPort = brokerPort;

#if OTA_ENABLED
  /* Calculate the SHA256 checksum over the firmware stored in the flash of the
   * MCU. Note: As we don't know the length per-se we read chunks of the flash
   * until we detect one containing only 0xFF (= flash erased). This only works
   * for firmware updated via OTA and second stage bootloaders (SxU family)
   * because only those erase the complete flash before performing an update.
   * Since the SHA256 firmware image is only required for the cloud servers to
   * perform a version check after the OTA update this is a acceptable trade off.
   * The bootloader is excluded from the calculation and occupies flash address
   * range 0 to 0x2000, total flash size of 0x40000 bytes (256 kByte).
   */
  _ota_img_sha256 = FlashSHA256::calc(0x2000, 0x40000 - 0x2000);
#endif /* OTA_ENABLED */

  #ifdef BOARD_HAS_ECCX08
  if (!ECCX08.begin())                                                                                                                                                         { DBG_ERROR("Cryptography processor failure. Make sure you have a compatible board."); return 0; }
  if (!CryptoUtil::readDeviceId(ECCX08, getDeviceId(), ECCX08Slot::DeviceId))                                                                                                  { DBG_ERROR("Cryptography processor read failure."); return 0; }
  if (!CryptoUtil::reconstructCertificate(_eccx08_cert, getDeviceId(), ECCX08Slot::Key, ECCX08Slot::CompressedCertificate, ECCX08Slot::SerialNumberAndAuthorityKeyIdentifier)) { DBG_ERROR("Cryptography certificate reconstruction failure."); return 0; }
  _sslClient.setClient(_connection->getClient());
  _sslClient.setEccSlot(static_cast<int>(ECCX08Slot::Key), _eccx08_cert.bytes(), _eccx08_cert.length());
  #elif defined(BOARD_ESP)
  _sslClient.setInsecure();
  #endif

  _mqttClient.setClient(_sslClient);
  #ifdef BOARD_ESP
  _mqttClient.setUsernamePassword(getDeviceId(), _password);
  #endif
  _mqttClient.onMessage(ArduinoIoTCloudTCP::onMessage);
  _mqttClient.setKeepAliveInterval(30 * 1000);
  _mqttClient.setConnectionTimeout(1500);
  _mqttClient.setId(getDeviceId().c_str());

  _stdinTopic     = getTopic_stdin();
  _stdoutTopic    = getTopic_stdout();
  _shadowTopicOut = getTopic_shadowout();
  _shadowTopicIn  = getTopic_shadowin();
  _dataTopicOut   = getTopic_dataout();
  _dataTopicIn    = getTopic_datain();
  _ota_topic_in   = getTopic_ota_in();

  printConnectionStatus(_iot_status);

#if   OTA_STORAGE_SSU
  setOTAStorage(ota_storage_ssu);
#elif OTA_STORAGE_SFU
  setOTAStorage(ota_storage_sfu);
#elif OTA_STORAGE_SNU
  setOTAStorage(ota_storage_snu);
#endif

  return 1;
}

void ArduinoIoTCloudTCP::update()
{
  if(checkPhyConnection()   != NetworkConnectionState::CONNECTED)     return;

  /* Retrieve the latest data from the MQTT Client. */
  if (_mqttClient.connected())
    _mqttClient.poll();

  /* Run through the state machine. */
  State next_state = _state;
  switch (_state)
  {
  case State::ConnectMqttBroker:   next_state = handle_ConnectMqttBroker();   break;
  case State::SubscribeMqttTopics: next_state = handle_SubscribeMqttTopics(); break;
  case State::RequestLastValues:   next_state = handle_RequestLastValues();   break;
  case State::Connected:           next_state = handle_Connected();           break;
  }
  _state = next_state;
}

int ArduinoIoTCloudTCP::connected()
{
  return _mqttClient.connected();
}

void ArduinoIoTCloudTCP::printDebugInfo()
{
  DBG_INFO("***** Arduino IoT Cloud - configuration info *****");
  DBG_INFO("Device ID: %s", getDeviceId().c_str());
  DBG_INFO("Thing ID: %s", getThingId().c_str());
  DBG_INFO("MQTT Broker: %s:%d", _brokerAddress.c_str(), _brokerPort);
}

#if OTA_ENABLED
void ArduinoIoTCloudTCP::setOTAStorage(OTAStorage & ota_storage)
{
  addPropertyReal(_ota_error, "OTA_ERROR", Permission::Read);
  addPropertyReal(_ota_img_sha256, "OTA_SHA256", Permission::Read);
  _ota_logic.setOTAStorage(ota_storage);
}
#endif /* OTA_ENABLED */

/******************************************************************************
 * PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_ConnectMqttBroker()
{
  if (_mqttClient.connect(_brokerAddress.c_str(), _brokerPort))
    return State::SubscribeMqttTopics;

  DBG_ERROR("ArduinoIoTCloudTCP::%s could not connect to %s:%d", __FUNCTION__, _brokerAddress.c_str(), _brokerPort);
  return State::ConnectMqttBroker;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_SubscribeMqttTopics()
{
  if (!_mqttClient.subscribe(_stdinTopic))
  {
    DBG_ERROR("ArduinoIoTCloudTCP::%s could not subscribe to %s", __FUNCTION__, _stdinTopic.c_str());
    return State::SubscribeMqttTopics;
  }

  if (!_mqttClient.subscribe(_dataTopicIn))
  {
    DBG_ERROR("ArduinoIoTCloudTCP::%s could not subscribe to %s", __FUNCTION__, _dataTopicIn.c_str());
    return State::SubscribeMqttTopics;
  }

  if (!_mqttClient.subscribe(_ota_topic_in))
  {
    DBG_ERROR("ArduinoIoTCloudTCP::%s could not subscribe to %s", __FUNCTION__, _ota_topic_in.c_str());
    return State::SubscribeMqttTopics;
  }

  if (_shadowTopicIn != "")
  {
    if (!_mqttClient.subscribe(_shadowTopicIn))
    {
      DBG_ERROR("ArduinoIoTCloudTCP::%s could not subscribe to %s", __FUNCTION__, _ota_topic_in.c_str());
      return State::SubscribeMqttTopics;
    }
  }

  DBG_VERBOSE("Connected to Arduino IoT Cloud");
  execCloudEventCallback(ArduinoIoTCloudEvent::CONNECT);
  return State::RequestLastValues;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_RequestLastValues()
{
  /* Check wether or not we need to send a new request. */
  unsigned long const now = millis();
  if ((now - _lastSyncRequestTickTime) > TIMEOUT_FOR_LASTVALUES_SYNC)
  {
    DBG_VERBOSE("ArduinoIoTCloudTCP::%s [%d] last values requested", __FUNCTION__, now);
    requestLastValue();
    _lastSyncRequestTickTime = now;
  }

  return State::RequestLastValues;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_Connected()
{
  if (!_mqttClient.connected())
  {
    /* The last message was definitely lost, trigger a retransmit. */
    _mqtt_data_request_retransmit = true;

    /* We are not connected anymore, trigger the callback for a disconnected event. */
    execCloudEventCallback(ArduinoIoTCloudEvent::DISCONNECT);

    /* Forcefully disconnect MQTT client and trigger a reconnection. */
    _mqttClient.stop();
    return State::ConnectMqttBroker;
  }
  /* We are connected so let's to our stuff here. */
  else
  {
#if OTA_ENABLED
    /* If a _ota_logic object has been instantiated then we are spinning its
     * 'update' method here in order to process incoming data and generally
     * to transition to the OTA logic update states.
     */
    OTAError const err = _ota_logic.update();
    _ota_error = static_cast<int>(err);
#endif /* OTA_ENABLED */

    /* Check if a primitive property wrapper is locally changed.
    * This function requires an existing time service which in
    * turn requires an established connection. Not having that
    * leads to a wrong time set in the time service which inhibits
    * the connection from being established due to a wrong data
    * in the reconstructed certificate.
    */
    updateTimestampOnLocallyChangedProperties(_property_container);

    /* Retransmit data in case there was a lost transaction due
    * to phy layer or MQTT connectivity loss.
    */
    if(_mqtt_data_request_retransmit && (_mqtt_data_len > 0)) {
      write(_dataTopicOut, _mqtt_data_buf, _mqtt_data_len);
      _mqtt_data_request_retransmit = false;
    }

    /* Check if any properties need encoding and send them to
    * the cloud if necessary.
    */
    sendPropertiesToCloud();

    return State::Connected;
  }
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

  if (_dataTopicIn == topic) {
    CBORDecoder::decode(_property_container, (uint8_t*)bytes, length);
  }

  if ((_shadowTopicIn == topic) && (_state == State::RequestLastValues))
  {
    DBG_VERBOSE("ArduinoIoTCloudTCP::%s [%d] last values received", __FUNCTION__, millis());
    CBORDecoder::decode(_property_container, (uint8_t*)bytes, length, true);
    sendPropertiesToCloud();
    execCloudEventCallback(ArduinoIoTCloudEvent::SYNC);
    _state = State::Connected;
  }

#if OTA_ENABLED
  if (_ota_topic_in == topic) {
    _ota_logic.onOTADataReceived(bytes, length);
  }
#endif /* OTA_ENABLED */
}

void ArduinoIoTCloudTCP::sendPropertiesToCloud()
{
  int bytes_encoded = 0;
  uint8_t data[MQTT_TRANSMIT_BUFFER_SIZE];

  if (CBOREncoder::encode(_property_container, data, sizeof(data), bytes_encoded, false) == CborNoError)
    if (bytes_encoded > 0)
    {
      /* If properties have been encoded store them in the back-up buffer
       * in order to allow retransmission in case of failure.
       */
      _mqtt_data_len = bytes_encoded;
      memcpy(_mqtt_data_buf, data, _mqtt_data_len);
      /* Transmit the properties to the MQTT broker */
      write(_dataTopicOut, _mqtt_data_buf, _mqtt_data_len);
    }
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

/******************************************************************************
 * EXTERN DEFINITION
 ******************************************************************************/

ArduinoIoTCloudTCP ArduinoCloud;

#endif
