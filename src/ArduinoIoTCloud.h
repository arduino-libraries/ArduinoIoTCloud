#ifndef ARDUINO_IOT_CLOUD_H
#define ARDUINO_IOT_CLOUD_H

#include <MQTT.h>
#include <ArduinoIoTCloudBearSSL.h>
#include <ArduinoCloudThing.h>

#include "CloudSerial.h"

typedef enum {
  READ      = 0x01,
  WRITE     = 0x02,
  READWRITE = READ | WRITE
} permissionType;

// Declaration of the struct for the mqtt connection options
typedef struct {
  int keepAlive;
  bool cleanSession;
  int timeout;
} mqttConnectionOptions;

class ArduinoIoTCloudClass {

public:
  ArduinoIoTCloudClass();
  ~ArduinoIoTCloudClass();

  int begin(Client& net, String brokerAddress = "mqtts-sa.iot.arduino.cc");

  // Class constant declaration
  static const int MQTT_BUFFER_SIZE = 256;
  static const int MAX_RETRIES = 5;
  static const int RECONNECTION_TIMEOUT = 2000;
  const mqttConnectionOptions mqttOpt = {30, false, 1500};

  int  connect   ();
  bool disconnect();

  void poll() __attribute__((deprecated)); /* Attention: Function is deprecated - use 'update' instead */
  void update();

  // defined for users who want to specify max reconnections reties and timeout between them
  void update(int const reconnectionMaxRetries, int const reconnectionTimeoutMs);
  // It must be a user defined function, in order to avoid ArduinoCloud include specific WiFi file
  // in this case this library is independent from the WiFi one
  void onGetTime(unsigned long(*)(void));

  int connected();
  // Clean up existing Mqtt connection, create a new one and initialize it
  int reconnect(Client& net);

  template<typename T, typename N=T>
  void addProperty(T & property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, N minDelta = N(0)) {
    Permission permission = Permission::ReadWrite;
    if     (permission_type == READ ) permission = Permission::Read;
    else if(permission_type == WRITE) permission = Permission::Write;
    else                              permission = Permission::ReadWrite;

    if(seconds == ON_CHANGE) {
      Thing.addProperty(property, name, permission).publishOnChange((T)minDelta).onUpdate(fn);
    }
    else {
      Thing.addProperty(property, name, permission).publishEvery(seconds).onUpdate(fn);
    }
  }

  template <typename T>
  ArduinoCloudProperty<T> addProperty(T & property, String const & name, Permission const permission) {
    return Thing.addProperty(property, name, permission);
  }

protected:
  friend class CloudSerialClass;
  int writeStdout(const byte data[], int const length);
  int writeProperties(const byte data[], int const length);
  // Used to initialize MQTTClient
  void mqttClientBegin(Client& net);
  // Function in charge of perform MQTT reconnection, basing on class parameters(retries,and timeout)
  bool mqttReconnect(int const maxRetries, int const timeout);

private:
  static void onMessage(MQTTClient *client, char topic[], char bytes[], int const length);
  void handleMessage(char topic[], char bytes[], int const length);

  String _id;
  String _brokerAddress;
  ArduinoCloudThing Thing;
  BearSSLClient* _bearSslClient;
  MQTTClient _mqttClient;

  // Class attribute to define MTTQ topics 2 for stdIn/out and 2 for data, in order to avoid getting previous pupblished payload
  String _stdinTopic;
  String _stdoutTopic;
  String _dataTopicOut;
  String _dataTopicIn;
  String _otaTopic;
};

extern ArduinoIoTCloudClass ArduinoCloud;

#endif
