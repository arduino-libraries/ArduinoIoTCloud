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

    template<typename T> void addPropertyReal(T& property, String name, permissionType _permission = READWRITE, long seconds = ON_CHANGE, T minDelta = 0, void(*fn)(void) = NULL) {
      Thing.addPropertyReal(property, name).publishEvery(seconds).setPermission(_permission).onUpdate(fn).minimumDelta(&minDelta);
    }

    void addPropertyReal(String& property, String name, permissionType _permission = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0) {
      Thing.addPropertyReal(property, name).publishEvery(seconds).setPermission(_permission).onUpdate(fn);
    }

    template<typename T> void addPropertyReal(T& property, String name, permissionType _permission = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, T minDelta = 0) {
      Thing.addPropertyReal(property, name).publishEvery(seconds).setPermission(_permission).onUpdate(fn).minimumDelta(&minDelta);
    }

    template<typename T> void addPropertyReal(T& property, String name, permissionType _permission = READWRITE, void(*fn)(void) = NULL, long seconds = ON_CHANGE, T minDelta = 0) { 
      Thing.addPropertyReal(property, name).publishEvery(seconds).setPermission(_permission).onUpdate(fn).minimumDelta(&minDelta);
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

    // Class attribute to define MTTQ topics 2 for stdIn/out and 2 for data, in order to avoid getting previous pupblished payload
    String _stdinTopic;
    String _stdoutTopic;
    String _dataTopicIn;
    String _dataTopicOut;
    String _otaTopic;
};

extern ArduinoCloudClass ArduinoCloud;

#endif
