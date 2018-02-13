#ifndef ARDUINO_CLOUD_V2
#define ARDUINO_CLOUD_V2

#include <MQTTClient.h>
#include <ArduinoBearSSL.h>

class ArduinoCloudClass {

public:
  ArduinoCloudClass();
  ~ArduinoCloudClass();

  int begin(Client& net, const String& id);

  int connect();

  void onGetTime(unsigned long(*)(void));

private:
  String _id;
  BearSSLClient* _bearSslClient;
  MQTTClient _mqttClient;
};


extern ArduinoCloudClass ArduinoCloud;

#endif
