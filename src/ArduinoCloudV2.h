#ifndef ARDUINO_CLOUD_V2_H
#define ARDUINO_CLOUD_V2_H

#include <MQTTClient.h>
#include <ArduinoBearSSL.h>

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

  int begin(Client& net);

  // Class constant declaration
  static const int MQTT_BUFFER_SIZE = 256;
  static const int MAX_RETRIES = 5;
  static const int RECONNECTION_TIMEOUT = 2000;
  const mqttConnectionOptions mqttOpt = {120, false, 1000};

  int connect();
  bool disconnect();
  void poll();

  // defined for users who want to specify max reconnections reties and timeout between them
  void poll(int reconnectionMaxRetries, int reconnectionTimeoutMs);
  // It must be a user defined function, in order to avoid ArduinoCloud include specific WiFi file
  // in this case this library is independent from the WiFi one
  void onGetTime(unsigned long(*)(void));

  int connected();
  // Clean up existing Mqtt connection, create a new one and initialize it
  void reconnect(Client& net);

protected:
  friend class CloudSerialClass;
  int writeStdout(const byte data[], int length);
  // Used to initialize MQTTClient
  void mqttClientBegin(Client& net);
  // Function in charge of perform MQTT reconnection, basing on class parameters(retries,and timeout)
  bool mqttReconnect(int maxRetries, int timeout);

private:
  static void onMessage(MQTTClient *client, char topic[], char bytes[], int length);
  void handleMessage(char topic[], char bytes[], int length);

  String _id;
  BearSSLClient* _bearSslClient;
  MQTTClient _mqttClient;

  // Class attribute to define MTTQ topics 2 for stdIn/out and 2 for data, in order to avoid getting previous pupblished payload
  String _stdinTopic;
  String _stdoutTopic;
};

extern ArduinoCloudClass ArduinoCloud;

#endif
