#include "ArduinoCloudV2.h"

#include <utility/ECCX08.h>
#include <utility/ECCX08Cert.h>

const static char server[] = "a19g5nbe27wn47.iot.eu-west-1.amazonaws.com"; //"xxxxxxxxxxxxxx.iot.xx-xxxx-x.amazonaws.com";

const static int keySlot            = 0;
const static int compressedCertSlot = 10;
const static int serialNumberSlot   = 11;

ArduinoCloudClass::ArduinoCloudClass() :
  _bearSslClient(NULL),
  _mqttClient(256)
{
}

ArduinoCloudClass::~ArduinoCloudClass()
{
  if (_bearSslClient) {
    delete _bearSslClient;
  }
}

int ArduinoCloudClass::begin(Client& net, const String& id)
{
  if (!ECCX08.begin()) {
    return 0;
  }

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

  _mqttClient.begin(server, 8883, *_bearSslClient);

  _id = id;

  return 1;
}

int ArduinoCloudClass::connect()
{
  if (!_mqttClient.connect(_id.c_str())) {
    return 0;
  }

  return 1;
}

void ArduinoCloudClass::poll()
{
  _mqttClient.loop();
}

void ArduinoCloudClass::onGetTime(unsigned long(*callback)(void))
{
  ArduinoBearSSL.onGetTime(callback);
}

ArduinoCloudClass ArduinoCloud;
