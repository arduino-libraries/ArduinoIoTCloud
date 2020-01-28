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

#ifdef HAS_TCP
#include <ArduinoIoTCloudTCP.h>
#ifdef BOARD_HAS_ECCX08
  #include "utility/ECCX08Cert.h"
  #include "utility/BearSSLTrustAnchor.h"
  #include <ArduinoECCX08.h>
#endif

#ifdef ARDUINO_ARCH_SAMD
  #include <RTCZero.h>
  RTCZero rtc;
#endif

#ifdef BOARD_HAS_ECCX08
  const static int keySlot									                 = 0;
  const static int compressedCertSlot						             = 10;
  const static int serialNumberAndAuthorityKeyIdentifierSlot = 11;
  const static int deviceIdSlot								               = 12;
#endif

const static int CONNECT_SUCCESS							               = 1;
const static int CONNECT_FAILURE							               = 0;
const static int CONNECT_FAILURE_SUBSCRIBE					         = -1;

static unsigned long getTime() {
  if (!ArduinoCloud.getConnection()) {
    return 0;
  }
  TcpIpConnectionHandler * connection = ArduinoCloud.getConnection();
  unsigned long time = connection->getTime();
  Debug.print(DBG_DEBUG, "NTP time: %lu", time);
  if (!NTPUtils::isTimeValid(time)) {
    Debug.print(DBG_ERROR, "Bogus NTP time from API, fallback to UDP method");
    time = NTPUtils(connection->getUDP()).getTime();
  }
  #ifdef ARDUINO_ARCH_SAMD
  rtc.setEpoch(time);
  #endif
  return time;
}

ArduinoIoTCloudTCP::ArduinoIoTCloudTCP():
  _connection(NULL),

  _sslClient(NULL),
  #ifdef BOARD_ESP
  _password(""),
  #endif
  _mqttClient(NULL),

  _stdinTopic(""),
  _stdoutTopic(""),
  _shadowTopicOut(""),
  _shadowTopicIn(""),
  _dataTopicOut(""),
  _dataTopicIn(""),
  _otaTopic("")
{}


ArduinoIoTCloudTCP::~ArduinoIoTCloudTCP() {
  if (_mqttClient) {
    delete _mqttClient;
    _mqttClient = NULL;
  }

  if (_sslClient) {
    delete _sslClient;
    _sslClient = NULL;
  }
}

int ArduinoIoTCloudTCP::begin(TcpIpConnectionHandler & connection, String brokerAddress, uint16_t brokerPort) {
  _connection = &connection;
  _brokerAddress = brokerAddress;
  _brokerPort = brokerPort;
  #ifdef ARDUINO_ARCH_SAMD
  rtc.begin();
  #endif
  return begin(_connection->getClient(), _brokerAddress, _brokerPort);
}

int ArduinoIoTCloudTCP::begin(Client& net, String brokerAddress, uint16_t brokerPort) {

  _net = &net;
  // store the broker address as class member
  _brokerAddress = brokerAddress;
  _brokerPort = brokerPort;

  #ifdef BOARD_HAS_ECCX08
  byte deviceIdBytes[72];
  if (!ECCX08.begin()) {
    Debug.print(DBG_ERROR, "Cryptography processor failure. Make sure you have a compatible board.");
    return 0;
  }

  if (!ECCX08.readSlot(deviceIdSlot, deviceIdBytes, sizeof(deviceIdBytes))) {
    Debug.print(DBG_ERROR, "Cryptography processor read failure.");
    return 0;
  }
  _device_id = (char*)deviceIdBytes;

  if (!ECCX08Cert.beginReconstruction(keySlot, compressedCertSlot, serialNumberAndAuthorityKeyIdentifierSlot)) {
    Debug.print(DBG_ERROR, "Cryptography certificate reconstruction failure.");
    return 0;
  }

  ECCX08Cert.setSubjectCommonName(_device_id);
  ECCX08Cert.setIssuerCountryName("US");
  ECCX08Cert.setIssuerOrganizationName("Arduino LLC US");
  ECCX08Cert.setIssuerOrganizationalUnitName("IT");
  ECCX08Cert.setIssuerCommonName("Arduino");

  if (!ECCX08Cert.endReconstruction()) {
    Debug.print(DBG_ERROR, "Cryptography certificate reconstruction failure.");
    return 0;
  }

  ArduinoBearSSL.onGetTime(getTime);
  #endif /* BOARD_HAS_ECCX08 */

  if (_sslClient) {
    delete _sslClient;
    _sslClient = NULL;
  }

  #ifdef BOARD_HAS_ECCX08
  if (_connection != NULL) {
    _sslClient = new BearSSLClient(_connection->getClient(), ArduinoIoTCloudTrustAnchor, ArduinoIoTCloudTrustAnchor_NUM);
  } else {
    _sslClient = new BearSSLClient(*_net, ArduinoIoTCloudTrustAnchor, ArduinoIoTCloudTrustAnchor_NUM);
  }
  _sslClient->setEccSlot(keySlot, ECCX08Cert.bytes(), ECCX08Cert.length());
  #elif defined(BOARD_ESP)
  _sslClient = new WiFiClientSecure();
  _sslClient->setInsecure();
  #endif

  _mqttClient = new MqttClient(*_sslClient);

  #ifdef BOARD_ESP
  _mqttClient->setUsernamePassword(_device_id, _password);
  #endif

  mqttClientBegin();

  Thing.begin();
  return 1;
}

// private class method used to initialize mqttClient class member. (called in the begin class method)
void ArduinoIoTCloudTCP::mqttClientBegin() {
  // MQTT topics definition
  _stdoutTopic = "/a/d/" + _device_id + "/s/o";
  _stdinTopic = "/a/d/" + _device_id + "/s/i";
  if (_thing_id == "") {
    _dataTopicIn = "/a/d/" + _device_id + "/e/i";
    _dataTopicOut = "/a/d/" + _device_id + "/e/o";
  } else {
    _dataTopicIn = "/a/t/" + _thing_id + "/e/i";
    _dataTopicOut = "/a/t/" + _thing_id + "/e/o";
    _shadowTopicIn = "/a/t/" + _thing_id + "/shadow/i";
    _shadowTopicOut = "/a/t/" + _thing_id + "/shadow/o";
  }

  // use onMessage as callback for received mqtt messages
  _mqttClient->onMessage(ArduinoIoTCloudTCP::onMessage);
  _mqttClient->setKeepAliveInterval(30 * 1000);
  _mqttClient->setConnectionTimeout(1500);
  _mqttClient->setId(_device_id.c_str());
}


int ArduinoIoTCloudTCP::connect() {

  if (!_mqttClient->connect(_brokerAddress.c_str(), _brokerPort)) {
    return CONNECT_FAILURE;
  }
  if (_mqttClient->subscribe(_stdinTopic) == 0) {
    return CONNECT_FAILURE_SUBSCRIBE;
  }
  if (_mqttClient->subscribe(_dataTopicIn) == 0) {
    return CONNECT_FAILURE_SUBSCRIBE;
  }
  if (_shadowTopicIn != "") {
    if (_mqttClient->subscribe(_shadowTopicIn) == 0) {
      return CONNECT_FAILURE_SUBSCRIBE;
    }

    _syncStatus = ArduinoIoTSynchronizationStatus::SYNC_STATUS_WAIT_FOR_CLOUD_VALUES;
    _lastSyncRequestTickTime = 0;
  }

  return CONNECT_SUCCESS;
}


bool ArduinoIoTCloudTCP::disconnect() {
  _mqttClient->stop();

  return true;
}

void ArduinoIoTCloudTCP::update(CallbackFunc onSyncCompleteCallback) {
  // Check if a primitive property wrapper is locally changed
  Thing.updateTimestampOnLocallyChangedProperties();

  connectionCheck();

  if (_iotStatus != ArduinoIoTConnectionStatus::CONNECTED) {
    return;
  }

  // MTTQClient connected!, poll() used to retrieve data from MQTT broker
  _mqttClient->poll();

  switch (_syncStatus) {
    case ArduinoIoTSynchronizationStatus::SYNC_STATUS_SYNCHRONIZED: {
        sendPropertiesToCloud();
      }
      break;
    case ArduinoIoTSynchronizationStatus::SYNC_STATUS_WAIT_FOR_CLOUD_VALUES: {
        if (millis() - _lastSyncRequestTickTime > TIMEOUT_FOR_LASTVALUES_SYNC) {
          requestLastValue();
          _lastSyncRequestTickTime = millis();
        }
      }
      break;
    case ArduinoIoTSynchronizationStatus::SYNC_STATUS_VALUES_PROCESSED: {
        if (onSyncCompleteCallback != NULL) {
          (*onSyncCompleteCallback)();
        }
        execCloudEventCallback(_on_sync_event_callback, 0 /* callback_arg */);
        _syncStatus = ArduinoIoTSynchronizationStatus::SYNC_STATUS_SYNCHRONIZED;
      }
      break;
  }
}



void ArduinoIoTCloudTCP::sendPropertiesToCloud() {
  uint8_t data[MQTT_TRANSMIT_BUFFER_SIZE];
  int const length = Thing.encode(data, sizeof(data));
  if (length > 0) {
    writeProperties(data, length);
  }
}


int ArduinoIoTCloudTCP::reconnect(Client& /* net */) {
  if (_mqttClient->connected()) {
    _mqttClient->stop();
  }

  // Connect to the broker
  return connect();
}


int ArduinoIoTCloudTCP::connected() {
  return _mqttClient->connected();
}

int ArduinoIoTCloudTCP::writeProperties(const byte data[], int length) {
  if (!_mqttClient->beginMessage(_dataTopicOut, length, false, 0)) {
    return 0;
  }

  if (!_mqttClient->write(data, length)) {
    return 0;
  }

  if (!_mqttClient->endMessage()) {
    return 0;
  }

  return 1;
}

int ArduinoIoTCloudTCP::writeStdout(const byte data[], int length) {
  if (!_mqttClient->beginMessage(_stdoutTopic, length, false, 0)) {
    return 0;
  }

  if (!_mqttClient->write(data, length)) {
    return 0;
  }

  if (!_mqttClient->endMessage()) {
    return 0;
  }

  return 1;
}

int ArduinoIoTCloudTCP::writeShadowOut(const byte data[], int length) {
  if (!_mqttClient->beginMessage(_shadowTopicOut, length, false, 0)) {
    return 0;
  }

  if (!_mqttClient->write(data, length)) {
    return 0;
  }

  if (!_mqttClient->endMessage()) {
    return 0;
  }

  return 1;
}

void ArduinoIoTCloudTCP::onMessage(int length) {
  ArduinoCloud.handleMessage(length);
}

void ArduinoIoTCloudTCP::handleMessage(int length) {
  String topic = _mqttClient->messageTopic();

  byte bytes[length];

  for (int i = 0; i < length; i++) {
    bytes[i] = _mqttClient->read();
  }

  if (_stdinTopic == topic) {
    CloudSerial.appendStdin((uint8_t*)bytes, length);
  }
  if (_dataTopicIn == topic) {
    Thing.decode((uint8_t*)bytes, length);
  }
  if ((_shadowTopicIn == topic) && _syncStatus == ArduinoIoTSynchronizationStatus::SYNC_STATUS_WAIT_FOR_CLOUD_VALUES) {
    Thing.decode((uint8_t*)bytes, length, true);
    sendPropertiesToCloud();
    _syncStatus = ArduinoIoTSynchronizationStatus::SYNC_STATUS_VALUES_PROCESSED;
  }
}

void ArduinoIoTCloudTCP::requestLastValue() {
  // Send the getLastValues CBOR message to the cloud
  // [{0: "r:m", 3: "getLastValues"}] = 81 A2 00 63 72 3A 6D 03 6D 67 65 74 4C 61 73 74 56 61 6C 75 65 73
  // Use http://cbor.me to easily generate CBOR encoding
  const uint8_t CBOR_REQUEST_LAST_VALUE_MSG[] = { 0x81, 0xA2, 0x00, 0x63, 0x72, 0x3A, 0x6D, 0x03, 0x6D, 0x67, 0x65, 0x74, 0x4C, 0x61, 0x73, 0x74, 0x56, 0x61, 0x6C, 0x75, 0x65, 0x73 };
  writeShadowOut(CBOR_REQUEST_LAST_VALUE_MSG, sizeof(CBOR_REQUEST_LAST_VALUE_MSG));
}

void ArduinoIoTCloudTCP::connectionCheck() {

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
        if (!_mqttClient->connected()) {
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
        int const ret_code_reconnect = reconnect(*_net);
        Debug.print(DBG_INFO, "ArduinoCloud.reconnect(): %d", ret_code_reconnect);
        if (ret_code_reconnect == CONNECT_SUCCESS) {
          _iotStatus = ArduinoIoTConnectionStatus::CONNECTED;
          printConnectionStatus(_iotStatus);
          execCloudEventCallback(_on_connect_event_callback, 0 /* callback_arg */);
          CloudSerial.begin(9600);
          CloudSerial.println("Hello from Cloud Serial!");
        }
      }
      break;
    case ArduinoIoTConnectionStatus::CONNECTING: {
        int const ret_code_connect = connect();
        Debug.print(DBG_VERBOSE, "ArduinoCloud.connect(): %d", ret_code_connect);
        if (ret_code_connect == CONNECT_SUCCESS) {
          _iotStatus = ArduinoIoTConnectionStatus::CONNECTED;
          printConnectionStatus(_iotStatus);
          execCloudEventCallback(_on_connect_event_callback, 0 /* callback_arg */);
          CloudSerial.begin(9600);
          CloudSerial.println("Hello from Cloud Serial!");
        } else if (ret_code_connect == CONNECT_FAILURE_SUBSCRIBE) {
          Debug.print(DBG_INFO, "ERROR - Please verify your THING ID");
        }
      }
      break;
  }
}

void ArduinoIoTCloudTCP::printDebugInfo() {
  Debug.print(DBG_INFO, "***** Arduino IoT Cloud - configuration info *****");
  Debug.print(DBG_INFO, "Device ID: %s", getDeviceId().c_str());
  Debug.print(DBG_INFO, "Thing ID: %s", getThingId().c_str());
  Debug.print(DBG_INFO, "MQTT Broker: %s:%d", _brokerAddress.c_str(), _brokerPort);
}

ArduinoIoTCloudTCP ArduinoCloud;

#endif