#ifndef ARDUINO_CLOUD_V2_H
#define ARDUINO_CLOUD_V2_H

#include <MQTTClient.h>
#include <ArduinoBearSSL.h>
#include <ArduinoCloudThing.h>
#include <ArduinoHttpClient.h>

#include "CloudSerial.h"

class ArduinoCloudClass {

public:
  ArduinoCloudClass();
  ~ArduinoCloudClass();

  int begin(Client& net);

  int connect();

  void poll();

  void onGetTime(unsigned long(*)(void));

  int connected();

  #define addProperty( v, ...) addPropertyReal(v, #v, __VA_ARGS__)

  template<typename T> void addPropertyReal(T property, String name, permissionType _permission = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL) {
    Thing.addPropertyReal(property, name).publishEvery(seconds).setPermission(_permission).onUpdate(fn);
  }

protected:
  friend class CloudSerialClass;
  int writeStdout(const byte data[], int length);
  int writeProperties(const byte data[], int length);

private:
  static void onMessage(MQTTClient *client, char topic[], char bytes[], int length);

  void handleMessage(char topic[], char bytes[], int length);

private:
  String _id;
  ArduinoCloudThing Thing;
  BearSSLClient* _bearSslClient;
  HttpClient* _otaClient;
  MQTTClient _mqttClient;

  String _stdinTopic;
  String _stdoutTopic;
  String _dataTopic;
  String _otaTopic;
};


extern ArduinoCloudClass ArduinoCloud;

#endif
