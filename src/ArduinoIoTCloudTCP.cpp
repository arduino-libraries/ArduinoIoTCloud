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

#ifdef BOARD_HAS_OFFLOADED_ECCX08
#include <ArduinoECCX08.h>
#include "tls/utility/CryptoUtil.h"
#endif

#if defined(ARDUINO_PORTENTA_H7_M7)
#  include "tls/utility/SHA256.h"
#  include <stm32h7xx_hal_rtc_ex.h>
#endif

#include "utility/ota/OTA.h"
#include "utility/ota/FlashSHA256.h"

#include "cbor/CBOREncoder.h"

/******************************************************************************
 * EXTERN
 ******************************************************************************/

#if defined(ARDUINO_PORTENTA_H7_M7)
extern RTC_HandleTypeDef RTCHandle;
#endif

/******************************************************************************
   GLOBAL CONSTANTS
 ******************************************************************************/

static const int TIMEOUT_FOR_LASTVALUES_SYNC = 10000;

/******************************************************************************
   LOCAL MODULE FUNCTIONS
 ******************************************************************************/

extern "C" unsigned long getTime()
{
  return ArduinoCloud.getInternalTime();
}

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

ArduinoIoTCloudTCP::ArduinoIoTCloudTCP()
: _state{State::ConnectPhy}
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
  _time_service.begin(&connection);
  return begin(_brokerAddress, _brokerPort);
}

int ArduinoIoTCloudTCP::begin(String brokerAddress, uint16_t brokerPort)
{
  _brokerAddress = brokerAddress;
  _brokerPort = brokerPort;

#if defined(__AVR__)
  String const nina_fw_version = WiFi.firmwareVersion();
  if (nina_fw_version < "1.4.2")
  {
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s NINA firmware needs to be >= 1.4.2 to support cloud on Uno WiFi Rev. 2, current %s", __FUNCTION__, nina_fw_version.c_str());
    return 0;
  }
#endif /* AVR */

#if OTA_ENABLED && !defined(__AVR__)
#if defined(ARDUINO_PORTENTA_H7_M7)
  /* The length of the application can be retrieved the same way it was
   * communicated to the bootloader, that is by writing to the non-volatile
   * storage registers of the RTC.
   */
  SHA256         sha256;
  uint32_t const app_start = 0x8040000;
  uint32_t const app_size  = HAL_RTCEx_BKUPRead(&RTCHandle, RTC_BKP_DR3);

  sha256.begin();
  uint32_t b = 0;
  uint32_t bytes_read = 0; for(uint32_t a = app_start;
                               bytes_read < app_size;
                               bytes_read += sizeof(b), a += sizeof(b))
  {
    /* Read the next chunk of memory. */
    memcpy(&b, reinterpret_cast<const void *>(a), sizeof(b));
    /* Feed it to SHA256. */
    sha256.update(reinterpret_cast<uint8_t *>(&b), sizeof(b));
  }
  /* Retrieve the final hash string. */
  uint8_t sha256_hash[SHA256::HASH_SIZE] = {0};
  sha256.finalize(sha256_hash);
  String sha256_str;
  std::for_each(sha256_hash,
                sha256_hash + SHA256::HASH_SIZE,
                [&sha256_str](uint8_t const elem)
                {
                  char buf[4];
                  snprintf(buf, 4, "%02X", elem);
                  sha256_str += buf;
                });
  DEBUG_VERBOSE("SHA256: %d bytes (of %d) read", bytes_read, app_size);
#else
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
  String const sha256_str = FlashSHA256::calc(0x2000, 0x40000 - 0x2000);
#endif
  DEBUG_VERBOSE("SHA256: HASH(%d) = %s", strlen(sha256_str.c_str()), sha256_str.c_str());
  _ota_img_sha256 = sha256_str;
#endif /* OTA_ENABLED */

  #ifdef BOARD_HAS_OFFLOADED_ECCX08
  if (!ECCX08.begin())
  {
    DEBUG_ERROR("ECCX08.begin() failed.");
    return 0;
  }
  if (!CryptoUtil::readDeviceId(ECCX08, getDeviceId(), ECCX08Slot::DeviceId))
  {
    DEBUG_ERROR("CryptoUtil::readDeviceId(...) failed.");
    return 0;
  }
  ECCX08.end();
  #endif

  #ifdef BOARD_HAS_ECCX08
  if (!ECCX08.begin())
  {
    DEBUG_ERROR("Cryptography processor failure. Make sure you have a compatible board.");
    return 0;
  }
  if (!CryptoUtil::readDeviceId(ECCX08, getDeviceId(), ECCX08Slot::DeviceId))
  {
    DEBUG_ERROR("Cryptography processor read failure.");
    return 0;
  }
  if (!CryptoUtil::reconstructCertificate(_eccx08_cert, getDeviceId(), ECCX08Slot::Key, ECCX08Slot::CompressedCertificate, ECCX08Slot::SerialNumberAndAuthorityKeyIdentifier))
  {
    DEBUG_ERROR("Cryptography certificate reconstruction failure.");
    return 0;
  }
  _sslClient.setClient(_connection->getClient());
  _sslClient.setEccSlot(static_cast<int>(ECCX08Slot::Key), _eccx08_cert.bytes(), _eccx08_cert.length());
  #elif defined(BOARD_ESP)
  #ifndef ESP32
  _sslClient.setInsecure();
  #endif
  #endif

  _mqttClient.setClient(_sslClient);
  #ifdef BOARD_ESP
  _mqttClient.setUsernamePassword(getDeviceId(), _password);
  #endif
  _mqttClient.onMessage(ArduinoIoTCloudTCP::onMessage);
  _mqttClient.setKeepAliveInterval(30 * 1000);
  _mqttClient.setConnectionTimeout(1500);
  _mqttClient.setId(getDeviceId().c_str());

  _shadowTopicOut = getTopic_shadowout();
  _shadowTopicIn  = getTopic_shadowin();
  _dataTopicOut   = getTopic_dataout();
  _dataTopicIn    = getTopic_datain();

#if OTA_ENABLED
  addPropertyReal(_ota_cap, "OTA_CAP", Permission::Read);
  addPropertyReal(_ota_error, "OTA_ERROR", Permission::Read);
  addPropertyReal(_ota_img_sha256, "OTA_SHA256", Permission::Read);
  addPropertyReal(_ota_url, "OTA_URL", Permission::ReadWrite).onSync(DEVICE_WINS);
  addPropertyReal(_ota_req, "OTA_REQ", Permission::ReadWrite).onSync(DEVICE_WINS);
#endif /* OTA_ENABLED */

#if OTA_STORAGE_PORTENTA_QSPI
  #define BOOTLOADER_ADDR   (0x8000000)
  uint32_t bootloader_data_offset = 0x1F000;
  uint8_t* bootloader_data = (uint8_t*)(BOOTLOADER_ADDR + bootloader_data_offset);
  uint8_t currentBootloaderVersion = bootloader_data[1];
  if (currentBootloaderVersion < 22) {
    _ota_cap = false;
    DEBUG_WARNING("ArduinoIoTCloudTCP::%s In order to be ready for cloud OTA, update the bootloader", __FUNCTION__);
    DEBUG_WARNING("ArduinoIoTCloudTCP::%s File -> Examples -> Portenta_System -> PortentaH7_updateBootloader", __FUNCTION__);
  }
  else {
    _ota_cap = true;
  }
#endif

#if OTA_STORAGE_SNU && OTA_ENABLED
  String const nina_fw_version = WiFi.firmwareVersion();
  if (nina_fw_version < "1.4.1") {
    _ota_cap = false;
    DEBUG_WARNING("ArduinoIoTCloudTCP::%s In order to be ready for cloud OTA, NINA firmware needs to be >= 1.4.1, current %s", __FUNCTION__, nina_fw_version.c_str());
  }
  else {
    _ota_cap = true;
  }
#endif /* OTA_STORAGE_SNU */

  return 1;
}

void ArduinoIoTCloudTCP::update()
{
  /* Run through the state machine. */
  State next_state = _state;
  switch (_state)
  {
  case State::ConnectPhy:          next_state = handle_ConnectPhy();          break;
  case State::SyncTime:            next_state = handle_SyncTime();            break;
  case State::ConnectMqttBroker:   next_state = handle_ConnectMqttBroker();   break;
  case State::SubscribeMqttTopics: next_state = handle_SubscribeMqttTopics(); break;
  case State::RequestLastValues:   next_state = handle_RequestLastValues();   break;
  case State::Connected:           next_state = handle_Connected();           break;
  }
  _state = next_state;

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
  DEBUG_INFO("Thing ID: %s", getThingId().c_str());
  DEBUG_INFO("MQTT Broker: %s:%d", _brokerAddress.c_str(), _brokerPort);
}

/******************************************************************************
 * PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_ConnectPhy()
{
  if (_connection->check() == NetworkConnectionState::CONNECTED)
    return State::SyncTime;
  else
    return State::ConnectPhy;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_SyncTime()
{
  unsigned long const internal_posix_time = _time_service.getTime();
  DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s internal clock configured to posix timestamp %d", __FUNCTION__, internal_posix_time);
  return State::ConnectMqttBroker;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_ConnectMqttBroker()
{
  if (_mqttClient.connect(_brokerAddress.c_str(), _brokerPort))
    return State::SubscribeMqttTopics;

  DEBUG_ERROR("ArduinoIoTCloudTCP::%s could not connect to %s:%d", __FUNCTION__, _brokerAddress.c_str(), _brokerPort);
  return State::ConnectPhy;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_SubscribeMqttTopics()
{
  if (!_mqttClient.connected())
  {
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s MQTT client connection lost", __FUNCTION__);
    _mqttClient.stop();
    execCloudEventCallback(ArduinoIoTCloudEvent::DISCONNECT);
    return State::ConnectPhy;
  }

  if (!_mqttClient.subscribe(_dataTopicIn))
  {
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s could not subscribe to %s", __FUNCTION__, _dataTopicIn.c_str());
#if !defined(__AVR__)
    DEBUG_ERROR("Check your thing configuration, and press the reset button on your board.");
#endif
    return State::SubscribeMqttTopics;
  }

  if (_shadowTopicIn != "")
  {
    if (!_mqttClient.subscribe(_shadowTopicIn))
    {
      DEBUG_ERROR("ArduinoIoTCloudTCP::%s could not subscribe to %s", __FUNCTION__, _shadowTopicIn.c_str());
#if !defined(__AVR__)
      DEBUG_ERROR("Check your thing configuration, and press the reset button on your board.");
#endif
      return State::SubscribeMqttTopics;
    }
  }

  DEBUG_INFO("Connected to Arduino IoT Cloud");
  execCloudEventCallback(ArduinoIoTCloudEvent::CONNECT);

  if (_shadowTopicIn != "")
    return State::RequestLastValues;
  else
    return State::Connected;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_RequestLastValues()
{
  if (!_mqttClient.connected())
  {
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s MQTT client connection lost", __FUNCTION__);
    _mqttClient.stop();
    execCloudEventCallback(ArduinoIoTCloudEvent::DISCONNECT);
    return State::ConnectPhy;
  }

  /* Check whether or not we need to send a new request. */
  unsigned long const now = millis();
  if ((now - _lastSyncRequestTickTime) > TIMEOUT_FOR_LASTVALUES_SYNC)
  {
    DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s [%d] last values requested", __FUNCTION__, now);
    requestLastValue();
    _lastSyncRequestTickTime = now;
  }

  return State::RequestLastValues;
}

ArduinoIoTCloudTCP::State ArduinoIoTCloudTCP::handle_Connected()
{
  if (!_mqttClient.connected())
  {
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s MQTT client connection lost", __FUNCTION__);

    /* Forcefully disconnect MQTT client and trigger a reconnection. */
    _mqttClient.stop();

    /* The last message was definitely lost, trigger a retransmit. */
    _mqtt_data_request_retransmit = true;

    /* We are not connected anymore, trigger the callback for a disconnected event. */
    execCloudEventCallback(ArduinoIoTCloudEvent::DISCONNECT);

    return State::ConnectPhy;
  }
  /* We are connected so let's to our stuff here. */
  else
  {
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

#if OTA_ENABLED
    /* Request a OTA download if the hidden property
     * OTA request has been set.
     */
    if (_ota_req)
    {
      /* Clear the error flag. */
      _ota_error = static_cast<int>(OTAError::None);
      /* Transmit the cleared error flag to the cloud. */
      sendPropertiesToCloud();
      /* Clear the request flag. */
      _ota_req = false;
      /* Call member function to handle OTA request. */
      onOTARequest();
    }
#endif /* OTA_ENABLED */

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
    DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s [%d] last values received", __FUNCTION__, millis());
    CBORDecoder::decode(_property_container, (uint8_t*)bytes, length, true);
    sendPropertiesToCloud();
    execCloudEventCallback(ArduinoIoTCloudEvent::SYNC);
    _state = State::Connected;
  }
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

#if OTA_ENABLED
void ArduinoIoTCloudTCP::onOTARequest()
{
  DEBUG_VERBOSE("ArduinoIoTCloudTCP::%s _ota_url = %s", __FUNCTION__, _ota_url.c_str());

#if OTA_STORAGE_SNU
  /* Just to be safe delete any remains from previous updates. */
  WiFiStorage.remove("/fs/UPDATE.BIN.LZSS");
  WiFiStorage.remove("/fs/UPDATE.BIN.LZSS.TMP");

  /* Trigger direct download to nina module. */
  uint8_t nina_ota_err_code = 0;
  if (!WiFiStorage.downloadOTA(_ota_url.c_str(), &nina_ota_err_code))
  {
    DEBUG_ERROR("ArduinoIoTCloudTCP::%s error download to nina: %d", __FUNCTION__, nina_ota_err_code);
    _ota_error = static_cast<int>(OTAError::DownloadFailed);
    return;
  }

  /* Perform the reset to reboot to SxU. */
  NVIC_SystemReset();
#endif /* OTA_STORAGE_SNU */

#if OTA_STORAGE_PORTENTA_QSPI
  Arduino_Portenta_OTA::Error ota_portenta_err = Arduino_Portenta_OTA::Error::None;
  /* Use 2nd partition of QSPI (1st partition contains WiFi firmware) */
  Arduino_Portenta_OTA_QSPI ota_portenta_qspi(QSPI_FLASH_FATFS_MBR, 2);

  /* Initialize the QSPI memory for OTA handling. */
  if((ota_portenta_err = ota_portenta_qspi.begin()) != Arduino_Portenta_OTA::Error::None) {
    DEBUG_ERROR("Arduino_Portenta_OTA_QSPI::begin() failed with %d", static_cast<int>(ota_portenta_err));
    return;
  }

  /* Just to be safe delete any remains from previous updates. */
  remove("/fs/UPDATE.BIN");
  remove("/fs/UPDATE.BIN.LZSS");

  /* Download the OTA file from the web storage location. */
  int const ota_portenta_qspi_download_ret_code = ota_portenta_qspi.download((char*)(_ota_url.c_str()), true /* is_https */);
  DEBUG_VERBOSE("Arduino_Portenta_OTA_QSPI::download(%s) returns %d", _ota_url.c_str(), ota_portenta_qspi_download_ret_code);

  /* Decompress the LZSS compressed OTA file. */
  int const ota_portenta_qspi_decompress_ret_code = ota_portenta_qspi.decompress();
  DEBUG_VERBOSE("Arduino_Portenta_OTA_QSPI::decompress() returns %d", ota_portenta_qspi_decompress_ret_code);
  if (ota_portenta_qspi_decompress_ret_code < 0)
  {
    DEBUG_ERROR("Arduino_Portenta_OTA_QSPI::decompress() failed with %d", ota_portenta_qspi_decompress_ret_code);
    return;
  }

  /* Schedule the firmware update. */
  if((ota_portenta_err = ota_portenta_qspi.update()) != Arduino_Portenta_OTA::Error::None) {
    DEBUG_ERROR("Arduino_Portenta_OTA_QSPI::update() failed with %d", static_cast<int>(ota_portenta_err));
    return;
  }

  /* Perform the reset to reboot - then the bootloader performs the actual application update. */
  NVIC_SystemReset();
#endif /* OTA_STORAGE_PORTENTA_QSPI */
}
#endif

/******************************************************************************
 * EXTERN DEFINITION
 ******************************************************************************/

ArduinoIoTCloudTCP ArduinoCloud;

#endif
