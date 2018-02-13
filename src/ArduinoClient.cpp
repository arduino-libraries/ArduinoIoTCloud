#include "ArduinoCloudV2.h"

#include <utility/ECC508.h>
#include <utility/ECC508Cert.h>

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
  if (!ECC508.begin()) {
    return 0;
  }

  if (!ECC508Cert.beginReconstruction(keySlot, compressedCertSlot, serialNumberSlot)) {
    return 0;
  }

  ECC508Cert.setSubjectCommonName(ECC508.serialNumber());
  ECC508Cert.setIssuerCountryName("US");
  ECC508Cert.setIssuerOrganizationName("Arduino LLC US");
  ECC508Cert.setIssuerOrganizationalUnitName("IT");
  ECC508Cert.setIssuerCommonName("Arduino");

  if (!ECC508Cert.endReconstruction()) {
    return 0;
  }

  if (_bearSslClient) {
    delete _bearSslClient;
  }
  _bearSslClient = new BearSSLClient(net);
  _bearSslClient->setEccSlot(keySlot, ECC508Cert.bytes(), ECC508Cert.length());

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

void ArduinoCloudClass::onGetTime(unsigned long(*callback)(void))
{
  ArduinoBearSSL.onGetTime(callback);
}

ArduinoCloudClass ArduinoCloud;
