
#include "ArduinoCloudV2.h"
#include <ArduinoECCX08.h>
#include <utility/ECCX08Cert.h>
#include "CloudSerial.h"

const static char server[] = "mqtts-sa.iot.oniudra.cc";

const static int keySlot                                   = 0;
const static int compressedCertSlot                        = 10;
const static int serialNumberAndAuthorityKeyIdentifierSlot = 11;
const static int thingIdSlot                               = 12;

ArduinoCloudClass::ArduinoCloudClass() :
  _bearSslClient(NULL),
  // Size of the receive buffer
  _mqttClient(MQTT_BUFFER_SIZE)
{
}

ArduinoCloudClass::~ArduinoCloudClass()
{
  if (_bearSslClient) {
    delete _bearSslClient;
  }
}

int ArduinoCloudClass::begin(Client& net)
{
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

  Serial.println("Compressed cert = ");

  const byte* certData = ECCX08Cert.bytes();
  int certLength = ECCX08Cert.length();

  for (int i = 0; i < certLength; i++) {
    byte b = certData[i];

    if (b < 16) {
      Serial.print('0');
    }
    Serial.print(b, HEX);
  }
  Serial.println();

  if (_bearSslClient) {
    delete _bearSslClient;
  }
  _bearSslClient = new BearSSLClient(net);
  _bearSslClient->setEccSlot(keySlot, ECCX08Cert.bytes(), ECCX08Cert.length());
  
  // Begin function for the MQTTClient
  mqttClientBegin(*_bearSslClient);
  // Thing initialization
  Thing.begin();

  return 1;
}

// private class method used to initialize mqttClient class member. (called in the begin class method)
void ArduinoCloudClass::mqttClientBegin(Client& net)
{
  // MQTT topics definition
  _stdoutTopic = "/a/d/" + _id + "/s/o";
  _stdinTopic = "/a/d/" + _id + "/s/i";
  _dataTopicIn = "/a/d/" + _id + "/e/i";
  _dataTopicOut = "/a/d/" + _id + "/e/o";

  // use onMessage as callback for received mqtt messages
  _mqttClient.onMessageAdvanced(ArduinoCloudClass::onMessage);
  _mqttClient.begin(server, 8883, net);
  // Set will for MQTT client: {topic, qos, retain message}
  const char lastMessage[] = "abcb";
  _mqttClient.setWill(_dataTopicOut.c_str(), lastMessage, false, 1);
  // Set MQTT connection options
  _mqttClient.setOptions(mqttOpt.keepAlive, mqttOpt.cleanSession, mqttOpt.timeout);
}

int ArduinoCloudClass::connect()
{
  //TODO MQTT brocker connection
  // Username: device id
  // Password: empty
  if (!_mqttClient.connect(_id.c_str())) {
    return 0;
  }
  _mqttClient.subscribe(_stdinTopic);
  _mqttClient.subscribe(_dataTopicIn);

  return 1;
}

bool ArduinoCloudClass::disconnect()
{
    return _mqttClient.disconnect();
}

void ArduinoCloudClass::poll()
{
  // If user call poll() without parameters use the default ones
  poll(MAX_RETRIES, RECONNECTION_TIMEOUT);
}

bool ArduinoCloudClass::mqttReconnect(int maxRetries, int timeout)
{
  // Counter for reconnection retries
  int retries = 0;

  // Check for MQTT broker connection, of if maxReties limit is reached
  // if MQTTClient is connected , simply do nothing and retun true
  while(!_mqttClient.connected() && retries++ < maxRetries) {

    // Get last MTTQClient error, (a common error may be a buffer overflow)
    lwmqtt_err_t err = _mqttClient.lastError();
 
    // try establish the MQTT broker connection 
    connect();
    // delay eventually used for the nex re-connection try
    delay(timeout);                   
  }

  // It was impossible to establish a connection, return
  if (retries == maxRetries) 
    return false;
  
  return true;
}

void ArduinoCloudClass::poll(int reconnectionMaxRetries, int reconnectionTimeoutMs)
{
  // Method's argument controls
  int maxRetries = (reconnectionMaxRetries > 0) ? reconnectionMaxRetries : MAX_RETRIES;
  int timeout = (reconnectionTimeoutMs > 0) ? reconnectionTimeoutMs : RECONNECTION_TIMEOUT;

  // If the reconnect() culd not establish the connection, return the control to the user sketch
  if (!mqttReconnect(maxRetries, timeout))
    return;

  // MTTQClient connected!, poll() used to retrieve data from MQTT broker
  _mqttClient.loop();

  uint8_t data[MQTT_BUFFER_SIZE];
  int length = Thing.poll(data, sizeof(data));
  // Are there some read properties that must be sent to the cloud ??
  if (length > 0) {
    // Check the connection is ok! if not reconnect
    // If a thing has all read properties only try to reconnect when the have to be sent(based on their update policy)
    writeProperties(data, length);
  }
}

void ArduinoCloudClass::reconnect(Client& net)
{
  // Initialize again the MQTTClient, otherwise it would not be able to receive messages through its callback
  mqttClientBegin(net);
  connect();
}

void ArduinoCloudClass::onGetTime(unsigned long(*callback)(void))
{
  ArduinoBearSSL.onGetTime(callback);
}

int ArduinoCloudClass::connected()
{
  return _mqttClient.connected();
}

int ArduinoCloudClass::writeProperties(const byte data[], int length)
{
  return _mqttClient.publish(_dataTopicOut.c_str(), (const char*)data, length);
}

int ArduinoCloudClass::writeStdout(const byte data[], int length)
{
  return _mqttClient.publish(_stdoutTopic.c_str(), (const char*)data, length);
}

void ArduinoCloudClass::onMessage(MQTTClient* /*client*/, char topic[], char bytes[], int length)
{
  ArduinoCloud.handleMessage(topic, bytes, length);
}

void ArduinoCloudClass::handleMessage(char topic[], char bytes[], int length)
{
  if (strcmp(_stdinTopic.c_str(), topic) == 0) {
    CloudSerial.appendStdin((uint8_t*)bytes, length);
  } 
  if (strcmp(_dataTopicIn.c_str(), topic) == 0) {
    Thing.decode((uint8_t*)bytes, length);
  }
}

ArduinoCloudClass ArduinoCloud;
