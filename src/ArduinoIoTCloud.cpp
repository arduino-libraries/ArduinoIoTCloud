/*
 * This file is part of ArduinoIoTCloud.
 *
 * Copyright 2019 ARDUINO SA (http://www.arduino.cc/)
 *
 * This software is released under the GNU General Public License version 3,
 * which covers the main part of arduino-cli.
 * The terms of this license can be found at:
 * https://www.gnu.org/licenses/gpl-3.0.en.html
 *
 * You can be released from the requirements of the above licenses by purchasing
 * a commercial license. Buying such a license is mandatory if you want to modify or
 * otherwise use the software for commercial activities involving the Arduino
 * software without disclosing the source code of your own applications. To purchase
 * a commercial license, send an email to license@arduino.cc.
 */

#include <ArduinoECCX08.h>
#include "utility/ECCX08Cert.h"
#include "CloudSerial.h"
#include "ArduinoIoTCloud.h"

#ifdef ARDUINO_ARCH_SAMD
  #include <RTCZero.h>
  RTCZero rtc;
#endif

const static int keySlot                                   = 0;
const static int compressedCertSlot                        = 10;
const static int serialNumberAndAuthorityKeyIdentifierSlot = 11;
const static int thingIdSlot                               = 12;

static ConnectionManager *getTimeConnection = NULL;

static unsigned long getTime() {
  if (!getTimeConnection) return 0;
  unsigned long time = getTimeConnection->getTime();
  if (!NTPUtils::isTimeValid(time)) {
    debugMessage("Bogus NTP time from API, fallback to UDP method", 0);
    time = NTPUtils(getTimeConnection->getUDP()).getTime();
  }
  return time;
}

static unsigned long getTimestamp() {
  #ifdef ARDUINO_ARCH_SAMD
    return rtc.getEpoch();
  #else
    #warning "No RTC available on this architecture - ArduinoIoTCloud will not keep track of local change timestamps ." 
    return 0;
  #endif
}

ArduinoIoTCloudClass::ArduinoIoTCloudClass() :
  _thing_id     (""),
  _bearSslClient(NULL),
  _mqttClient   (NULL),
  connection    (NULL),
  _lastSyncRequestTickTime(0)
{
}

ArduinoIoTCloudClass::~ArduinoIoTCloudClass()
{
  if (_mqttClient) {
    delete _mqttClient;
    _mqttClient = NULL;
  }

  if (_bearSslClient) {
    delete _bearSslClient;
    _bearSslClient = NULL;
  }
}

int ArduinoIoTCloudClass::begin(ConnectionManager *c, String brokerAddress, uint16_t brokerPort)
{
  connection = c;
  Client &connectionClient = c->getClient();
  _brokerAddress = brokerAddress;
  _brokerPort = brokerPort;
  #ifdef ARDUINO_ARCH_SAMD
    rtc.begin();
  #endif
  return begin(connectionClient, _brokerAddress, _brokerPort);
}

int ArduinoIoTCloudClass::begin(Client& net, String brokerAddress, uint16_t brokerPort)
{

  _net = &net;
  // store the broker address as class member
  _brokerAddress = brokerAddress;
  _brokerPort = brokerPort;
  byte thingIdBytes[72];

  if (!ECCX08.begin()) {
    debugMessage("Cryptography processor failure. Make sure you have a compatible board.", 0);
    return 0;
  }

  if (!ECCX08.readSlot(thingIdSlot, thingIdBytes, sizeof(thingIdBytes))) {
    debugMessage("Cryptography processor read failure.", 0);
    return 0;
  }
  _id = (char*)thingIdBytes;

  if (!ECCX08Cert.beginReconstruction(keySlot, compressedCertSlot, serialNumberAndAuthorityKeyIdentifierSlot)) {
    debugMessage("Cryptography certificate reconstruction failure.", 0);
    return 0;
  }

  ECCX08Cert.setSubjectCommonName(_id);
  ECCX08Cert.setIssuerCountryName("US");
  ECCX08Cert.setIssuerOrganizationName("Arduino LLC US");
  ECCX08Cert.setIssuerOrganizationalUnitName("IT");
  ECCX08Cert.setIssuerCommonName("Arduino");

  if (!ECCX08Cert.endReconstruction()) {
    debugMessage("Cryptography certificate reconstruction failure.", 0);
    return 0;
  }

  if (_bearSslClient) {
    delete _bearSslClient;
  }
  if(connection != NULL){
    _bearSslClient = new BearSSLClient(connection->getClient());
  }else{
    _bearSslClient = new BearSSLClient(*_net);
  }
  
  _bearSslClient->setEccSlot(keySlot, ECCX08Cert.bytes(), ECCX08Cert.length());
  _mqttClient = new MqttClient(*_bearSslClient);

  // Bind ArduinoBearSSL callback using static "non-method" function
  if(connection != NULL){
    getTimeConnection = connection;
    ArduinoBearSSL.onGetTime(getTime);
  }
  
  
  // TODO: Find a better way to allow callback into object method
  // Begin function for the MQTTClient
  mqttClientBegin();

  Thing.begin();
  return 1;
}

void ArduinoIoTCloudClass::onGetTime(unsigned long(*callback)(void))
{
  ArduinoBearSSL.onGetTime(callback);
}

// private class method used to initialize mqttClient class member. (called in the begin class method)
void ArduinoIoTCloudClass::mqttClientBegin()
{
  // MQTT topics definition
  _stdoutTopic  = "/a/d/" + _id + "/s/o";
  _stdinTopic   = "/a/d/" + _id + "/s/i";
  if(_thing_id == "") {
    _dataTopicIn  = "/a/d/" + _id + "/e/i";
    _dataTopicOut = "/a/d/" + _id + "/e/o";
  }
  else {
    _dataTopicIn  = "/a/t/" + _thing_id + "/e/i";
    _dataTopicOut = "/a/t/" + _thing_id + "/e/o";
    _shadowTopicIn  = "/a/t/" + _thing_id + "/shadow/i";
    _shadowTopicOut = "/a/t/" + _thing_id + "/shadow/o";
  }

  // use onMessage as callback for received mqtt messages
  _mqttClient->onMessage(ArduinoIoTCloudClass::onMessage);
  _mqttClient->setKeepAliveInterval(30 * 1000);
  _mqttClient->setConnectionTimeout(1500);
  _mqttClient->setId(_id.c_str());
}

int ArduinoIoTCloudClass::connect()
{
  // Username: device id
  // Password: empty
  if (!_mqttClient->connect(_brokerAddress.c_str(), _brokerPort)) {
    return 0;
  }
  _mqttClient->subscribe(_stdinTopic);
  _mqttClient->subscribe(_dataTopicIn);
  _mqttClient->subscribe(_shadowTopicIn);

  _syncStatus = ArduinoIoTSynchronizationStatus::SYNC_STATUS_WAIT_FOR_CLOUD_VALUES;
  _lastSyncRequestTickTime = 0;

  return 1;
}

bool ArduinoIoTCloudClass::disconnect()
{
  _mqttClient->stop();

  return true;
}

void ArduinoIoTCloudClass::poll()
{
  update();
}

void ArduinoIoTCloudClass::update(CallbackFunc onSyncCompleteCallback)
{
  // If user call update() without parameters use the default ones
  update(MAX_RETRIES, RECONNECTION_TIMEOUT, onSyncCompleteCallback);
}

void ArduinoIoTCloudClass::update(int const reconnectionMaxRetries, int const reconnectionTimeoutMs, CallbackFunc onSyncCompleteCallback)
{
  unsigned long const timestamp = getTimestamp(); 
  //check if a property is changed 
  if(timestamp != 0) Thing.updateTimestampOnChangedProperties(timestamp);
    
  connectionCheck();
  if(iotStatus != IOT_STATUS_CLOUD_CONNECTED){
    return;
  }

  // MTTQClient connected!, poll() used to retrieve data from MQTT broker
  _mqttClient->poll();

  switch (_syncStatus) {
    case ArduinoIoTSynchronizationStatus::SYNC_STATUS_SYNCHRONIZED:
      sendPropertiesToCloud();
      break;
    case ArduinoIoTSynchronizationStatus::SYNC_STATUS_WAIT_FOR_CLOUD_VALUES:
      if (millis() - _lastSyncRequestTickTime > TIMEOUT_FOR_LASTVALUES_SYNC) {
        requestLastValue();
        _lastSyncRequestTickTime = millis();
      }
      break;
    case ArduinoIoTSynchronizationStatus::SYNC_STATUS_VALUES_PROCESSED:
      if(onSyncCompleteCallback != NULL)
        (*onSyncCompleteCallback)();
      _syncStatus = ArduinoIoTSynchronizationStatus::SYNC_STATUS_SYNCHRONIZED;
      break;
  }
}

void ArduinoIoTCloudClass::sendPropertiesToCloud()
{
  uint8_t data[MQTT_TRANSMIT_BUFFER_SIZE];
  int const length = Thing.encode(data, sizeof(data));
  if (length > 0) {
    writeProperties(data, length);
  }
}

int ArduinoIoTCloudClass::reconnect(Client& /* net */)
{
  if (_mqttClient->connected()) {
    _mqttClient->stop();
  }

  // Connect to the broker
  return connect();
}

int ArduinoIoTCloudClass::connected()
{
  return _mqttClient->connected();
}

int ArduinoIoTCloudClass::writeProperties(const byte data[], int length)
{
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

int ArduinoIoTCloudClass::writeStdout(const byte data[], int length)
{
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

int ArduinoIoTCloudClass::writeShadowOut(const byte data[], int length)
{
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

void ArduinoIoTCloudClass::onMessage(int length)
{
  ArduinoCloud.handleMessage(length);
}

void ArduinoIoTCloudClass::handleMessage(int length)
{
  String topic = _mqttClient->messageTopic();

  byte bytes[length];
  int index = 0;

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

void ArduinoIoTCloudClass::requestLastValue()
{
  // Send the getLastValues CBOR message to the cloud
  // [{0: "r:m", 3: "getLastValues"}] = 81 A2 00 63 72 3A 6D 03 6D 67 65 74 4C 61 73 74 56 61 6C 75 65 73
  // Use http://cbor.me to easily generate CBOR encoding
  const uint8_t CBOR_REQUEST_LAST_VALUE_MSG[] =  { 0x81, 0xA2, 0x00, 0x63, 0x72, 0x3A, 0x6D, 0x03, 0x6D, 0x67, 0x65, 0x74, 0x4C, 0x61, 0x73, 0x74, 0x56, 0x61, 0x6C, 0x75, 0x65, 0x73 };
  writeShadowOut(CBOR_REQUEST_LAST_VALUE_MSG, sizeof(CBOR_REQUEST_LAST_VALUE_MSG));
}

void ArduinoIoTCloudClass::connectionCheck()
{
  if(connection != NULL){
    connection->check();
    
    if (connection->getStatus() != CONNECTION_STATE_CONNECTED) {
      if(iotStatus == IOT_STATUS_CLOUD_CONNECTED){
        setIoTConnectionState(IOT_STATUS_CLOUD_DISCONNECTED);
      }
      return;
    }
  }
  
  char msgBuffer[120];
  

  switch (iotStatus) {
    case IOT_STATUS_CLOUD_IDLE:
      setIoTConnectionState(IOT_STATUS_CLOUD_CONNECTING);
      break;
    case IOT_STATUS_CLOUD_ERROR:
      debugMessage("Cloud Error. Retrying...", 0);
      setIoTConnectionState(IOT_STATUS_CLOUD_RECONNECTING);
      break;
    case IOT_STATUS_CLOUD_CONNECTED:
      debugMessage(".", 4, false, true);
      if (!_mqttClient->connected()){
        setIoTConnectionState(IOT_STATUS_CLOUD_DISCONNECTED);
      }
      break;
    case IOT_STATUS_CLOUD_DISCONNECTED:
      setIoTConnectionState(IOT_STATUS_CLOUD_RECONNECTING);
      break;
    case IOT_STATUS_CLOUD_RECONNECTING:
      int arduinoIoTReconnectionAttempt;
      arduinoIoTReconnectionAttempt = reconnect(*_net);
      sprintf(msgBuffer, "ArduinoCloud.reconnect(): %d", arduinoIoTReconnectionAttempt);
      debugMessage(msgBuffer, 2);
      if (arduinoIoTReconnectionAttempt == 1) {
        setIoTConnectionState(IOT_STATUS_CLOUD_CONNECTED);
        CloudSerial.begin(9600);
        CloudSerial.println("Hello from Cloud Serial!");
      }
      break;
    case IOT_STATUS_CLOUD_CONNECTING:
      int arduinoIoTConnectionAttempt;
      arduinoIoTConnectionAttempt = connect();
      sprintf(msgBuffer, "ArduinoCloud.connect(): %d", arduinoIoTConnectionAttempt);
      debugMessage(msgBuffer, 4);
      if (arduinoIoTConnectionAttempt == 1) {
        setIoTConnectionState(IOT_STATUS_CLOUD_CONNECTED);
        CloudSerial.begin(9600);
        CloudSerial.println("Hello from Cloud Serial!");
      }
      #ifdef ARDUINO_ARCH_SAMD
        unsigned long const epoch = getTime();
        if (epoch!=0)
          rtc.setEpoch(epoch);
      #endif
      break;
  }
}

void ArduinoIoTCloudClass::setIoTConnectionState(ArduinoIoTConnectionStatus _newState)
{
  switch(_newState){
    case IOT_STATUS_CLOUD_ERROR:
      debugMessage("Arduino, we have a problem.", 0);
      break;
    case IOT_STATUS_CLOUD_CONNECTING:
      debugMessage("Connecting to Arduino IoT Cloud...", 0);
      break;
    case IOT_STATUS_CLOUD_RECONNECTING:
      debugMessage("Reconnecting to Arduino IoT Cloud...", 0);
      break;
    case IOT_STATUS_CLOUD_CONNECTED:
      debugMessage("Connected to Arduino IoT Cloud", 0);
      break;
    case IOT_STATUS_CLOUD_DISCONNECTED:
      debugMessage("Disconnected from Arduino IoT Cloud", 0);
      break;
  }
  iotStatus = _newState;
}

void ArduinoIoTCloudClass::printDebugInfo()
{
  char msgBuffer[120];
  debugMessage("***** Arduino IoT Cloud - configuration info *****", 2);
  sprintf(msgBuffer, "Device ID: %s", getDeviceId().c_str());
  debugMessage(msgBuffer, 2);
  sprintf(msgBuffer, "Thing ID: %s", getThingId().c_str());
  debugMessage(msgBuffer, 2);
  sprintf(msgBuffer, "MQTT Broker: %s:%d", _brokerAddress.c_str(), _brokerPort);
  debugMessage(msgBuffer, 2);
}

ArduinoIoTCloudClass ArduinoCloud;
