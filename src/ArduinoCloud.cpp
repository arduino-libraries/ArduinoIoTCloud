#include "ArduinoCloudV2.h"

#include <utility/ECCX08.h>
#include <utility/ECCX08Cert.h>

#include "CloudSerial.h"
#include "SerialFlashStorage.h"
#include "SFU.h"

const static char server[] = "a19g5nbe27wn47.iot.us-east-1.amazonaws.com"; //"xxxxxxxxxxxxxx.iot.xx-xxxx-x.amazonaws.com";

const static int keySlot            = 0;
const static int compressedCertSlot = 10;
const static int serialNumberSlot   = 11;
const static int thingIdSlot        = 12;

ArduinoCloudClass::ArduinoCloudClass() :
  _bearSslClient(NULL),
  _otaClient(NULL),
  _mqttClient(256)
{
}

ArduinoCloudClass::~ArduinoCloudClass()
{
  if (_bearSslClient) {
    delete _bearSslClient;
  }
  if (_otaClient) {
    delete _otaClient;
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

  if (!ECCX08Cert.beginReconstruction(keySlot, compressedCertSlot, serialNumberSlot)) {
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

  _mqttClient.onMessageAdvanced(ArduinoCloudClass::onMessage);
  _mqttClient.begin(server, 8883, *_bearSslClient);

  Thing.begin();

  _otaClient = new HttpClient(net, server, 80);

  _stdoutTopic = "$aws/things/" + _id + "/stdout";
  _stdinTopic = "$aws/things/" + _id + "/stdin";
  _dataTopic = "$aws/things/" + _id + "/data";
  _otaTopic = "$aws/things/" + _id + "/upload";

  return 1;
}

int ArduinoCloudClass::connect()
{
  if (!_mqttClient.connect(_id.c_str())) {
    return 0;
  }

  _mqttClient.subscribe(_stdinTopic);
  _mqttClient.subscribe(_dataTopic);
  _mqttClient.subscribe(_otaTopic);

  return 1;
}

void ArduinoCloudClass::poll()
{
  _mqttClient.loop();
  uint8_t data[1024];
  int length = Thing.poll(data);
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
  return _mqttClient.publish(_dataTopic.c_str(), (const char*)data, length);
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
  if (_stdinTopic == topic) {
    CloudSerial.appendStdin((uint8_t*)bytes, length);
  }
  if (_dataTopic == topic) {
    Thing.decode((uint8_t*)bytes, length);
  }
  if (_otaTopic == topic) {

    String url = String(bytes);

    _otaClient->get(url);

    SerialFlashStorage.open(_otaClient->contentLength());
    uint8_t buf[1024];
    while (_otaClient->available()) {
      int size = _otaClient->available() >= 1024 ? 1024 : _otaClient->available();
      _otaClient->read(buf, size);
      SerialFlashStorage.write((uint8_t*)buf, size);
    }
    SerialFlashStorage.close();
    SerialFlashStorage.apply();
  }
}

ArduinoCloudClass ArduinoCloud;
