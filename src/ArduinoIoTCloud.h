/*
   This file is part of ArduinoIoTCloud.

   Copyright 2019 ARDUINO SA (http://www.arduino.cc/)

   This software is released under the GNU General Public License version 3,
   which covers the main part of arduino-cli.
   The terms of this license can be found at:
   https://www.gnu.org/licenses/gpl-3.0.en.html

   You can be released from the requirements of the above licenses by purchasing
   a commercial license. Buying such a license is mandatory if you want to modify or
   otherwise use the software for commercial activities involving the Arduino
   software without disclosing the source code of your own applications. To purchase
   a commercial license, send an email to license@arduino.cc.
*/

#ifndef ARDUINO_IOT_CLOUD_H
#define ARDUINO_IOT_CLOUD_H

#include <ArduinoIoTCloud_Defines.h>

#ifdef BOARD_HAS_ECCX08
  #include <ArduinoBearSSL.h>
#elif defined(BOARD_ESP)
  #include <WiFiClientSecure.h>
#endif

#include <ArduinoCloudThing.h>
#include <ArduinoMqttClient.h>
#include <Arduino_DebugUtils.h>
#include <Arduino_ConnectionHandler.h>
#include "types/CloudWrapperBool.h"
#include "types/CloudWrapperFloat.h"
#include "types/CloudWrapperInt.h"
#include "types/CloudWrapperString.h"
#include "utility/NTPUtils.h"

#include "CloudSerial.h"

static char const DEFAULT_BROKER_ADDRESS_SECURE_AUTH[] = "mqtts-sa.iot.arduino.cc";
static uint16_t const DEFAULT_BROKER_PORT_SECURE_AUTH = 8883;
static char const DEFAULT_BROKER_ADDRESS_USER_PASS_AUTH[] = "mqtts-up.iot.arduino.cc";
static uint16_t const DEFAULT_BROKER_PORT_USER_PASS_AUTH = 8884;

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

enum class ArduinoIoTConnectionStatus {
  IDLE,
  CONNECTING,
  CONNECTED,
  DISCONNECTED,
  RECONNECTING,
  ERROR,
};

enum class ArduinoIoTSynchronizationStatus {
  SYNC_STATUS_SYNCHRONIZED,
  SYNC_STATUS_WAIT_FOR_CLOUD_VALUES,
  SYNC_STATUS_VALUES_PROCESSED
};

enum class ArduinoIoTCloudEvent {
  SYNC, CONNECT, DISCONNECT
};

typedef void (*CallbackFunc)(void);
typedef void (*OnCloudEventCallback)(void * /* arg */);

class ArduinoIoTCloudClass {

  public:
    ArduinoIoTCloudClass();
    ~ArduinoIoTCloudClass();

    #ifdef BOARD_HAS_ECCX08
    int begin(ConnectionHandler &connection, String brokerAddress = DEFAULT_BROKER_ADDRESS_SECURE_AUTH, uint16_t brokerPort = DEFAULT_BROKER_PORT_SECURE_AUTH);
    #else
    int begin(ConnectionHandler &connection, String brokerAddress = DEFAULT_BROKER_ADDRESS_USER_PASS_AUTH, uint16_t brokerPort = DEFAULT_BROKER_PORT_USER_PASS_AUTH);
    #endif
    int begin(Client &net, String brokerAddress = DEFAULT_BROKER_ADDRESS_SECURE_AUTH, uint16_t brokerPort = DEFAULT_BROKER_PORT_SECURE_AUTH);
    // Class constant declaration
    static const int MQTT_TRANSMIT_BUFFER_SIZE = 256;
    static const int TIMEOUT_FOR_LASTVALUES_SYNC = 10000;

    int  connect();
    bool disconnect();

    inline void update() {
      update(NULL);
    }
    inline void update(int const reconnectionMaxRetries, int const reconnectionTimeoutMs) __attribute__((deprecated)) {
      update(NULL);
    }
    void update(CallbackFunc onSyncCompleteCallback) __attribute__((deprecated)); /* Attention: Function is deprecated - use 'addCallback(ArduinoIoTCloudConnectionEvent::SYNC, &onSync)' for adding a onSyncCallback instead */

    int connected();
    // Clean up existing Mqtt connection, create a new one and initialize it
    int reconnect(Client& /* net */);

    inline void setThingId(String const thing_id) {
      _thing_id = thing_id;
    };
    #ifdef BOARD_ESP
    inline void setBoardId(String const device_id) {
      _device_id = device_id;
    }
    inline void setSecretDeviceKey(String const password) {
      _password = password;
    }
    #endif
    inline String getThingId()  const {
      return _thing_id;
    };
    inline String getDeviceId() const {
      return _device_id;
    };
    inline ConnectionHandler * getConnection() {
      return _connection;
    }

#define addProperty( v, ...) addPropertyReal(v, #v, __VA_ARGS__)

    static unsigned long const DEFAULT_MIN_TIME_BETWEEN_UPDATES_MILLIS = 500; /* Data rate throttled to 2 Hz */

    void addPropertyReal(ArduinoCloudProperty& property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(ArduinoCloudProperty& property) = CLOUD_WINS) {
      Permission permission = Permission::ReadWrite;
      if (permission_type == READ) {
        permission = Permission::Read;
      } else if (permission_type == WRITE) {
        permission = Permission::Write;
      } else {
        permission = Permission::ReadWrite;
      }

      if (seconds == ON_CHANGE) {
        Thing.addPropertyReal(property, name, permission).publishOnChange(minDelta, DEFAULT_MIN_TIME_BETWEEN_UPDATES_MILLIS).onUpdate(fn).onSync(synFn);
      } else {
        Thing.addPropertyReal(property, name, permission).publishEvery(seconds).onUpdate(fn).onSync(synFn);
      }
    }
    void addPropertyReal(bool& property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS) {
      ArduinoCloudProperty *p = new CloudWrapperBool(property);
      addPropertyReal(*p, name, permission_type, seconds, fn, minDelta, synFn);
    }
    ArduinoCloudProperty& addPropertyReal(bool& property, String name, Permission const permission) {
      ArduinoCloudProperty *p = new CloudWrapperBool(property);
      return Thing.addPropertyReal(*p, name, permission);
    }
    void addPropertyReal(float& property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS) {
      ArduinoCloudProperty *p = new CloudWrapperFloat(property);
      addPropertyReal(*p, name, permission_type, seconds, fn, minDelta, synFn);
    }
    ArduinoCloudProperty& addPropertyReal(float& property, String name, Permission const permission) {
      ArduinoCloudProperty *p = new CloudWrapperFloat(property);
      return Thing.addPropertyReal(*p, name, permission);
    }
    void addPropertyReal(int& property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS) {
      ArduinoCloudProperty *p = new CloudWrapperInt(property);
      addPropertyReal(*p, name, permission_type, seconds, fn, minDelta, synFn);
    }
    ArduinoCloudProperty& addPropertyReal(int& property, String name, Permission const permission) {
      ArduinoCloudProperty *p = new CloudWrapperInt(property);
      return Thing.addPropertyReal(*p, name, permission);
    }
    void addPropertyReal(String& property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS) {
      ArduinoCloudProperty *p = new CloudWrapperString(property);
      addPropertyReal(*p, name, permission_type, seconds, fn, minDelta, synFn);
    }
    ArduinoCloudProperty& addPropertyReal(String& property, String name, Permission const permission) {
      ArduinoCloudProperty *p = new CloudWrapperString(property);
      return Thing.addPropertyReal(*p, name, permission);
    }

    void connectionCheck();
    String getBrokerAddress() {
      return _brokerAddress;
    }
    uint16_t getBrokerPort() {
      return _brokerPort;
    }
    void printDebugInfo();
    void addCallback(ArduinoIoTCloudEvent const event, OnCloudEventCallback callback);

  protected:
    friend class CloudSerialClass;
    int writeStdout(const byte data[], int length);
    int writeProperties(const byte data[], int length);
    int writeShadowOut(const byte data[], int length);

    // Used to initialize MQTTClient
    void mqttClientBegin();
    // Function in charge of perform MQTT reconnection, basing on class parameters(retries,and timeout)
    bool mqttReconnect(int const maxRetries, int const timeout);
    // Used to retrieve last values from _shadowTopicIn
    void requestLastValue();

    ArduinoIoTConnectionStatus getIoTStatus() {
      return _iotStatus;
    }

  private:
    ArduinoIoTConnectionStatus _iotStatus = ArduinoIoTConnectionStatus::IDLE;
    ConnectionHandler * _connection;
    static void onMessage(int length);
    void handleMessage(int length);
    ArduinoIoTSynchronizationStatus _syncStatus = ArduinoIoTSynchronizationStatus::SYNC_STATUS_SYNCHRONIZED;

    void sendPropertiesToCloud();

    String _device_id, _thing_id, _brokerAddress;
    uint16_t _brokerPort;

    ArduinoCloudThing Thing;

    #ifdef BOARD_HAS_ECCX08
    BearSSLClient *_sslClient;
    #elif defined(BOARD_ESP)
    WiFiClientSecure *_sslClient;
    String _password;
    #endif

    MqttClient *_mqttClient;
    int _lastSyncRequestTickTime;


    // Class attribute to define MTTQ topics 2 for stdIn/out and 2 for data, in order to avoid getting previous pupblished payload
    String _stdinTopic;
    String _stdoutTopic;
    String _shadowTopicOut;
    String _shadowTopicIn;
    String _dataTopicOut;
    String _dataTopicIn;
    String _otaTopic;
    Client *_net;

    OnCloudEventCallback _on_sync_event_callback,
                         _on_connect_event_callback,
                         _on_disconnect_event_callback;

    static void execCloudEventCallback(OnCloudEventCallback & callback, void * callback_arg);
    static void printConnectionStatus(ArduinoIoTConnectionStatus status);
};

extern ArduinoIoTCloudClass ArduinoCloud;

#endif
