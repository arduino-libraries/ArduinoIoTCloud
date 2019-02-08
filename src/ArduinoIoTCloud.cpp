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
  return getTimeConnection->getTime();
}

static unsigned long getTimestamp() {
  #ifdef ARDUINO_ARCH_SAMD
    unsigned long epoch = getTime();
    if (epoch!=0){
      rtc.setEpoch(epoch);
      return epoch;
    } else {
      return rtc.getEpoch();
    }
  #else
    return 0;
  #endif
}

ArduinoIoTCloudClass::ArduinoIoTCloudClass() :
  _thing_id     (""),
  _bearSslClient(NULL),
  _mqttClient   (NULL),
  _firstReconnectionUpdate(false),
  _propertiesSynchronized(true),
  _syncCallback(NULL),
  _mode(PROPERTIES_SYNC_FORCE_DEVICE),
  _check_lastValues_sync(MAX_CHECK_LASTVALUES_SYNC),
  connection    (NULL),
  _callGetLastValueCallback(false)
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

int ArduinoIoTCloudClass::begin(ConnectionManager *c, String brokerAddress)
{
  connection = c;
  Client &connectionClient = c->getClient();
  _brokerAddress = brokerAddress;
  #ifdef ARDUINO_ARCH_SAMD
    rtc.begin();
  #endif
  return begin(connectionClient, _brokerAddress);
}

int ArduinoIoTCloudClass::begin(Client& net, String brokerAddress)
{

  _net = &net;
  // store the broker address as class member
  _brokerAddress = brokerAddress;
  byte thingIdBytes[72];

  if (!ECCX08.begin()) {
    return 0;
  }

  if (!ECCX08.readSlot(thingIdSlot, thingIdBytes, sizeof(thingIdBytes))) {
    return 0;
  }
  _id = (char*)thingIdBytes;

  if (!ECCX08Cert.beginReconstruction(keySlot, compressedCertSlot, serialNumberAndAuthorityKeyIdentifierSlot)) {
    return 0;
  }

  ECCX08Cert.setSubjectCommonName(_id);
  ECCX08Cert.setIssuerCountryName("US");
  ECCX08Cert.setIssuerOrganizationName("Arduino LLC US");
  ECCX08Cert.setIssuerOrganizationalUnitName("IT");
  ECCX08Cert.setIssuerCommonName("Arduino");

  if (!ECCX08Cert.endReconstruction()) {
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
  if (!_mqttClient->connect(_brokerAddress.c_str(), 8883)) {
    return 0;
  }
  _mqttClient->subscribe(_stdinTopic);
  _mqttClient->subscribe(_dataTopicIn);
  _mqttClient->subscribe(_shadowTopicIn);

  // setting the flag of first connectio/reconnection
  _firstReconnectionUpdate = true;

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

void ArduinoIoTCloudClass::update(int mode, void (*callback)(void))
{
  // If user call update() without parameters use the default ones
  update(MAX_RETRIES, RECONNECTION_TIMEOUT, mode, callback);
}

bool ArduinoIoTCloudClass::mqttReconnect(int const maxRetries, int const timeout)
{
  // Counter for reconnection retries
  int retries = 0;
  unsigned long start = millis();

  // Check for MQTT broker connection, of if maxReties limit is reached
  // if MQTTClient is connected , simply do nothing and retun true
  while (!_mqttClient->connected() && (retries++ < maxRetries) && (millis() - start < timeout)) {
    // int connectError = _mqttClient->connectError();

    // try establish the MQTT broker connection
    connect();
  }

  // It was impossible to establish a connection, return
  if ((retries == maxRetries) || (millis() - start >= timeout))
    return false;

  return true;
}

void ArduinoIoTCloudClass::update(int const reconnectionMaxRetries, int const reconnectionTimeoutMs, int mode, void (*callback)(void))
{
  _mode = mode;
  unsigned long timestamp = getTimestamp(); 
  //check if a property is changed 
  if(timestamp) Thing.updateTimestampOnChangedProperties(timestamp);
    
  connectionCheck();
  if(iotStatus != IOT_STATUS_CLOUD_CONNECTED){
    return;
  }
  // Method's argument controls
  int const maxRetries = (reconnectionMaxRetries > 0) ? reconnectionMaxRetries : MAX_RETRIES;
  int const timeout    = (reconnectionTimeoutMs  > 0) ? reconnectionTimeoutMs  : RECONNECTION_TIMEOUT;

  // If the reconnect() culd not establish the connection, return the control to the user sketch
  if (!mqttReconnect(maxRetries, timeout))
    return;

  // MTTQClient connected!, poll() used to retrieve data from MQTT broker
  _mqttClient->poll();

  //if getLastValues response is not reeceived after MAX_CHECK_LASTVALUES_SYNC attempts, resend getLastValues request
  if(!_propertiesSynchronized) {
    if(_check_lastValues_sync--){
      _firstReconnectionUpdate = true;
      _propertiesSynchronized = true;
    }
  } 

  if(((_mode == PROPERTIES_SYNC_FORCE_CLOUD) || (_mode == PROPERTIES_SYNC_AUTO)) && _firstReconnectionUpdate){
    // get the last values of the shadow_properties from MQTT broker
    getLastValues();
    // properties will be not synchronized until the OnMessage callback will be activated on _shadowTopicIn
    _firstReconnectionUpdate = false;
    _propertiesSynchronized = false;
    _check_lastValues_sync = MAX_CHECK_LASTVALUES_SYNC;
    // store callback pointer to the function to be called after the sync process end
    _syncCallback = callback;
  } 

  if(_mode == PROPERTIES_SYNC_FORCE_DEVICE){
    _firstReconnectionUpdate = false;
  } 

  // If in the properties are synchronized or the synch mode is PROPERTIES_SYNC_FORCE_DEVICE, send values to MQTT broker
  if((_propertiesSynchronized) && !(_mode == CLOUDSERIAL_SYNC)){
    uint8_t data[MQTT_TRANSMIT_BUFFER_SIZE];
    int const length = Thing.encode(data, sizeof(data));
    if (length > 0) {
      writeProperties(data, length);
    }
  }

  if(_callGetLastValueCallback) {
    if(_syncCallback != NULL) {
      (*_syncCallback)();
    }
    _callGetLastValueCallback = false;
  }
}

bool ArduinoIoTCloudClass::getLastValuesSyncStatus() {
  return _propertiesSynchronized;
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
  if ((_shadowTopicIn == topic) && !_propertiesSynchronized) {

    Thing.decode((uint8_t*)bytes, length, _mode);
    // set the flag to let know the update cycle that the properties are synchronized and the sync have been made followinf the _mode rules
    _propertiesSynchronized = true;
    // delay execution of LastValueCallback at the end of the update() cycle
    _callGetLastValueCallback = true;
  }
}

void ArduinoIoTCloudClass::getLastValues()
{
    uint8_t data[MQTT_TRANSMIT_BUFFER_SIZE];
    int const length = Thing.getLastValues(data, sizeof(data));
    if (length > 0) {
      writeShadowOut(data, length);
    }
}

void ArduinoIoTCloudClass::connectionCheck() {
  if(connection != NULL){
    connection->check();
    
    if (connection->getStatus() != CONNECTION_STATE_CONNECTED) {
      if(iotStatus == IOT_STATUS_CLOUD_CONNECTED){
        iotStatus = IOT_STATUS_CLOUD_DISCONNECTED;
      }else{
        iotStatus = IOT_STATUS_CLOUD_CONNECTING;  
      }
      return;
    }
  }
  
  char msgBuffer[120];
  

  switch (iotStatus) {
    case IOT_STATUS_IDLE:
      if(connection == NULL){
        if(!begin(*_net, _brokerAddress)){
          debugMessage("Error Starting Arduino Cloud\nTrying again in a few seconds", 0);
          iotStatus = IOT_STATUS_CLOUD_ERROR;
          return;
        }
      }else{
        if (!begin(connection, _brokerAddress)) {
          debugMessage("Error Starting Arduino Cloud\nTrying again in a few seconds", 0);
          iotStatus = IOT_STATUS_CLOUD_ERROR;
          return;
        }
      }
      
      iotStatus = IOT_STATUS_CLOUD_CONNECTING;
      break;
    case IOT_STATUS_CLOUD_ERROR:
      debugMessage("Cloud Error. Retrying...", 0);
      iotStatus = IOT_STATUS_CLOUD_RECONNECTING;
      break;
    case IOT_STATUS_CLOUD_CONNECTED:
      debugMessage("connected to Arduino IoT Cloud", 3);
      break;
    case IOT_STATUS_CLOUD_DISCONNECTED:
      debugMessage("disconnected from Arduino IoT Cloud", 0);
      iotStatus = IOT_STATUS_CLOUD_RECONNECTING;
      break;
    case IOT_STATUS_CLOUD_RECONNECTING:
      debugMessage("IoT Cloud reconnecting...", 1);
      //wifiClient.stop();
      int arduinoIoTReconnectionAttempt;
      arduinoIoTReconnectionAttempt = reconnect(*_net);
      *msgBuffer = 0;
      sprintf(msgBuffer, "ArduinoCloud.reconnect(): %d", arduinoIoTReconnectionAttempt);
      debugMessage(msgBuffer, 1);
      if (arduinoIoTReconnectionAttempt == 1) {
        iotStatus = IOT_STATUS_CLOUD_CONNECTED;
        CloudSerial.begin(9600);
        CloudSerial.println("Hello from Cloud Serial!");
      }
      break;
    case IOT_STATUS_CLOUD_CONNECTING:
      debugMessage("IoT Cloud connecting...", 1);
      int arduinoIoTConnectionAttempt;
      arduinoIoTConnectionAttempt = connect();
      *msgBuffer = 0;
      sprintf(msgBuffer, "ArduinoCloud.connect(): %d", arduinoIoTConnectionAttempt);
      debugMessage(msgBuffer, 2);
      if (arduinoIoTConnectionAttempt == 1) {
        iotStatus = IOT_STATUS_CLOUD_CONNECTED;
        CloudSerial.begin(9600);
        CloudSerial.println("Hello from Cloud Serial!");
      }
      break;
  }
}

ArduinoIoTCloudClass ArduinoCloud;
