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

#include <ArduinoMqttClient.h>
#include <ArduinoIoTCloudBearSSL.h>
#include <ArduinoCloudThing.h>
#include "ConnectionManager.h"
#include "types/CloudWrapperBool.h"
#include "types/CloudWrapperFloat.h"
#include "types/CloudWrapperInt.h"
#include "types/CloudWrapperString.h"


#include "CloudSerial.h"

#define DEFAULT_BROKER_ADDRESS "mqtts-sa.iot.arduino.cc"
#define DEFAULT_BROKER_PORT 8883
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

    int begin(ConnectionManager *connection = ArduinoIoTPreferredConnection, String brokerAddress = DEFAULT_BROKER_ADDRESS, uint16_t brokerPort = DEFAULT_BROKER_PORT);
    int begin(Client& net, String brokerAddress = DEFAULT_BROKER_ADDRESS, uint16_t brokerPort = DEFAULT_BROKER_PORT);
    // Class constant declaration
    static const int MQTT_TRANSMIT_BUFFER_SIZE = 256;
    static const int TIMEOUT_FOR_LASTVALUES_SYNC = 10000;

    void onGetTime(unsigned long(*callback)(void));

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

    inline String getThingId()  const {
      return _thing_id;
    };
    inline String getDeviceId() const {
      return _device_id;
    };

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
      return iotStatus;
    }
    void setIoTConnectionState(ArduinoIoTConnectionStatus newState);
  private:
    ArduinoIoTConnectionStatus iotStatus = ArduinoIoTConnectionStatus::IDLE;
    ConnectionManager * _connection;
    static void onMessage(int length);
    void handleMessage(int length);
    ArduinoIoTSynchronizationStatus _syncStatus = ArduinoIoTSynchronizationStatus::SYNC_STATUS_SYNCHRONIZED;

    void sendPropertiesToCloud();


    String _device_id,
           _thing_id,
           _brokerAddress;
    uint16_t _brokerPort;
    ArduinoCloudThing Thing;
    BearSSLClient* _bearSslClient;
    MqttClient* _mqttClient;
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
};

extern ArduinoIoTCloudClass ArduinoCloud;

#endif
