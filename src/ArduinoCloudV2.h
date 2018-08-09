#ifndef ARDUINO_CLOUD_V2_H
#define ARDUINO_CLOUD_V2_H

#include <MQTTClient.h>
#include <ArduinoBearSSL.h>
#include <ArduinoCloudThing.h>
#include <ArduinoHttpClient.h>

#include "CloudSerial.h"

// Declaration of the struct for the mqtt connection options
typedef struct mqtt_opt{
    int keepAlive;
    bool cleanSession;
    int timeout;
} mqttConnectionOptions;

class ArduinoCloudClass {

public:
    ArduinoCloudClass();
    ~ArduinoCloudClass();

    // Class constant declaration
    static const int MQTT_BUFFER_SIZE = 256;
    static const int MAX_RETRIES = 5; 
    static const int RECONNECTION_TIMEOUT = 2000;
    const mqttConnectionOptions mqttOpt = {120, false, 1000};

    int begin(Client& net);
    int connect();
    bool disconnect();
    void poll();
    // defined for users who want to specify max reconnections reties and timeout between them
    void poll(int reconnectionMaxRetries, int reconnectionTimeoutMs);
    // It must be a user defined function, in order to avoid ArduinoCloud include specificW iFi file
    // in this case this library is independent from the WiFi one
    void onGetTime(unsigned long(*)(void));
    int connected();
    // Clean up existing Mqtt connection, create a new one and initialize it
    void reconnect(Client& net);

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
    // Used to initialize MQTTClient
    void mqttClientBegin(Client& net);
    // Function in charge of perform MQTT reconnection, basing on class parameters(retries,and timeout)
    bool mqttReconnect(int maxRetries, int timeout);

private:
    static void onMessage(MQTTClient *client, char topic[], char bytes[], int length);
    void handleMessage(char topic[], char bytes[], int length);

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
