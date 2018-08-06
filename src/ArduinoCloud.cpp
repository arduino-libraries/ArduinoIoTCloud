
#include "ArduinoCloudV2.h"
#include <ECCX08.h>
#include <utility/ECCX08Cert.h>
#include "CloudSerial.h"

const static char server[] = "broker.shiftr.io"; //"a19g5nbe27wn47.iot.us-east-1.amazonaws.com"; //"xxxxxxxxxxxxxx.iot.xx-xxxx-x.amazonaws.com";

const static int keySlot                                   = 0;
const static int compressedCertSlot                        = 10;
const static int serialNumberAndAuthorityKeyIdentifierSlot = 11;
const static int thingIdSlot                               = 12;

ArduinoCloudClass::ArduinoCloudClass() :
  _bearSslClient(NULL),
  // Size of the receive buffer
  _mqttClient(256)
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
  
  /*
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

  ECCX08Cert.setSubjectCommonName(ECCX08.serialNumber());
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
  _bearSslClient = new BearSSLClient(net);
  _bearSslClient->setEccSlot(keySlot, ECCX08Cert.bytes(), ECCX08Cert.length());
*/
  //END of TLS communication part. The result of that part is *_bearSslClient [Network Client]
  
  
  // MQTT topics definition
  _id = "XXX";

  _stdoutTopic = "/a/d/" + _id + "/s/o";
  _stdinTopic = "/a/d/" + _id + "/s/i";
  _dataTopicIn = "/a/d/" + _id + "/e/i";
  _dataTopicOut = "/a/d/" + _id + "/e/o";

  // use onMessage as callback for received mqtt messages
  _mqttClient.onMessageAdvanced(ArduinoCloudClass::onMessage);
  //_mqttClient.begin(server, 8883, *_bearSslClient);
  _mqttClient.begin(server, 1883, net);
  // Set will for MQTT client: {topic, qos, retain message}
  const char lastMessage[] = "abcb";
  _mqttClient.setWill(_dataTopicOut.c_str(), lastMessage, false, 1);
  // Set MQTT broker connection options
  _mqttClient.setOptions(120, false, 1000);

  // Thing initialization
  Thing.begin();

  return 1;
}

int ArduinoCloudClass::connect()
{
  //TODO MQTT brocker connection
  // Username: device id
  // Password: empty
  if (!_mqttClient.connect(_id.c_str(), "try", "try")) {
    return 0;
  }

  _mqttClient.subscribe(_stdinTopic);
  _mqttClient.subscribe(_dataTopicIn);

  return 1;
}

void ArduinoCloudClass::poll()
{
  _mqttClient.loop();
  uint8_t data[1024];
  int length = Thing.poll(data, sizeof(data));
  if (length > 0) {
    writeProperties(data, length);
  }
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
