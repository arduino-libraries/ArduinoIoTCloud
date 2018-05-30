#ifndef ARDUINO_CLOUD_V2_H
#define ARDUINO_CLOUD_V2_H

#include <MQTTClient.h>
#include <ArduinoBearSSL.h>
#include <ArduinoCloudThing.h>

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

  #define addPropertyMacro(prop) addPropertyReal(prop, #prop)
  #undef addProperty

  template<typename T> void addProperty(T property, permissionType _permission = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL) {
    Thing.addPropertyMacro(property).publishEvery(seconds).setPermission(_permission).onUpdate(fn);
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
  MQTTClient _mqttClient;

  String _stdinTopic;
  String _stdoutTopic;
  String _dataTopic;
};


extern ArduinoCloudClass ArduinoCloud;

#endif
