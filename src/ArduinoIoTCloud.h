#ifndef ARDUINO_IOT_CLOUD_H
#define ARDUINO_IOT_CLOUD_H

#include <ArduinoMqttClient.h>
#include <ArduinoIoTCloudBearSSL.h>
#include <ArduinoCloudThing.h>
#include "ConnectionManager.h"

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

extern ConnectionManager *ArduinoIoTPreferredConnection;

enum ArduinoIoTConnectionStatus {
  IOT_STATUS_IDLE,/* only at start */
  IOT_STATUS_CLOUD_IDLE,
  IOT_STATUS_CLOUD_CONNECTING,
  IOT_STATUS_CLOUD_CONNECTED,
  IOT_STATUS_CLOUD_DISCONNECTED,
  IOT_STATUS_CLOUD_RECONNECTING,
  IOT_STATUS_CLOUD_ERROR,
  IOT_STATUS_NETWORK_IDLE,
  IOT_STATUS_NETWORK_CONNECTED,
  IOT_STATUS_NETWORK_CONNECTING,
  IOT_STATUS_NETWORK_DISCONNECTED,
  IOT_STATUS_NETWORK_ERROR,
  IOT_STATUS_ERROR_GENERIC
};

class ArduinoIoTCloudClass {

public:
  ArduinoIoTCloudClass();
  ~ArduinoIoTCloudClass();

  int begin(ConnectionManager *connection = ArduinoIoTPreferredConnection, String brokerAddress = "mqtts-sa.iot.arduino.cc");

  // Class constant declaration
  static const int MQTT_TRANSMIT_BUFFER_SIZE = 256;
  static const int MAX_RETRIES = 5;
  static const int RECONNECTION_TIMEOUT = 2000;

  int  connect   ();
  bool disconnect();

  void poll() __attribute__((deprecated)); /* Attention: Function is deprecated - use 'update' instead */
  void update();

  // defined for users who want to specify max reconnections reties and timeout between them
  void update(int const reconnectionMaxRetries, int const reconnectionTimeoutMs);

  int connected();
  // Clean up existing Mqtt connection, create a new one and initialize it
  int reconnect();

  inline void setThingId(String const thing_id) { _thing_id = thing_id; };

  inline String getThingId()  const { return _thing_id; };
  inline String getDeviceId() const { return _id; };

  #define addProperty( v, ...) addPropertyReal(v, #v, __VA_ARGS__)

  static unsigned long const DEFAULT_MIN_TIME_BETWEEN_UPDATES_MILLIS = 100; /* Data rate throttled to 10 Hz */

  template<typename T, typename N=T>
  void addPropertyReal(T & property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, N minDelta = N(0)) {
    Permission permission = Permission::ReadWrite;
    if     (permission_type == READ ) permission = Permission::Read;
    else if(permission_type == WRITE) permission = Permission::Write;
    else                              permission = Permission::ReadWrite;

    if(seconds == ON_CHANGE) {
      Thing.addPropertyReal(property, name, permission).publishOnChange((T)minDelta, DEFAULT_MIN_TIME_BETWEEN_UPDATES_MILLIS).onUpdate(fn);
    }
    else {
      Thing.addPropertyReal(property, name, permission).publishEvery(seconds).onUpdate(fn);
    }
  }

  template <typename T>
  ArduinoCloudProperty<T> addPropertyReal(T & property, String const & name, Permission const permission) {
    return Thing.addPropertyReal(property, name, permission);
  }

  void connectionCheck();

protected:
  friend class CloudSerialClass;
  int writeStdout(const byte data[], int length);
  int writeProperties(const byte data[], int length);
  // Used to initialize MQTTClient
  void mqttClientBegin();
  // Function in charge of perform MQTT reconnection, basing on class parameters(retries,and timeout)
  bool mqttReconnect(int const maxRetries, int const timeout);

  ArduinoIoTConnectionStatus getIoTStatus() { return iotStatus; }

private:
  ArduinoIoTConnectionStatus iotStatus = IOT_STATUS_IDLE;
  ConnectionManager *connection;
  static void onMessage(int length);
  void handleMessage(int length);

  String _id,
         _thing_id,
         _brokerAddress;
  ArduinoCloudThing Thing;
  BearSSLClient* _bearSslClient;
  MqttClient* _mqttClient;

  // Class attribute to define MTTQ topics 2 for stdIn/out and 2 for data, in order to avoid getting previous pupblished payload
  String _stdinTopic;
  String _stdoutTopic;
  String _dataTopicOut;
  String _dataTopicIn;
  String _otaTopic;
};

extern ArduinoIoTCloudClass ArduinoCloud;

#endif
