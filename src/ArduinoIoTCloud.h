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

typedef void (*CallbackFunc)(void);

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

class ArduinoIoTCloudClass {

  public:
    ArduinoIoTCloudClass();
    ~ArduinoIoTCloudClass();

    int begin(ConnectionManager *connection = ArduinoIoTPreferredConnection, String brokerAddress = DEFAULT_BROKER_ADDRESS, uint16_t brokerPort = DEFAULT_BROKER_PORT);
    int begin(Client& net, String brokerAddress = DEFAULT_BROKER_ADDRESS, uint16_t brokerPort = DEFAULT_BROKER_PORT);
    // Class constant declaration
    static const int MQTT_TRANSMIT_BUFFER_SIZE = 256;
    static const int MAX_RETRIES = 5;
    static const int RECONNECTION_TIMEOUT = 2000;
    static const int TIMEOUT_FOR_LASTVALUES_SYNC = 10000;

    void onGetTime(unsigned long(*callback)(void));

    int  connect();
    bool disconnect();

    void update(CallbackFunc onSyncCompleteCallback = NULL);

    // defined for users who want to specify max reconnections reties and timeout between them
    void update(int const reconnectionMaxRetries, int const reconnectionTimeoutMs, CallbackFunc onSyncCompleteCallback = NULL);

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



    template<typename T, typename N = T>
    void addPropertyReal(T & property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, N minDelta = N(0), void(*synFn)(ArduinoCloudProperty<T> property) = CLOUD_WINS) {
      Permission permission = Permission::ReadWrite;
      if (permission_type == READ) {
        permission = Permission::Read;
      } else if (permission_type == WRITE) {
        permission = Permission::Write;
      } else {
        permission = Permission::ReadWrite;
      }

      if (seconds == ON_CHANGE) {
        Thing.addPropertyReal(property, name, permission).publishOnChange((T)minDelta, DEFAULT_MIN_TIME_BETWEEN_UPDATES_MILLIS).onUpdate(fn).onSync(synFn);
      } else {
        Thing.addPropertyReal(property, name, permission).publishEvery(seconds).onUpdate(fn).onSync(synFn);
      }
    }

    template <typename T>
    ArduinoCloudProperty<T> addPropertyReal(T & property, String const & name, Permission const permission) {
      return Thing.addPropertyReal(property, name, permission);
    }

    void connectionCheck();
    String getBrokerAddress() {
      return _brokerAddress;
    }
    uint16_t getBrokerPort() {
      return _brokerPort;
    }
    void printDebugInfo();

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
    void setIoTConnectionState(ArduinoIoTConnectionStatus _newState);
  private:
    ArduinoIoTConnectionStatus iotStatus = ArduinoIoTConnectionStatus::IDLE;
    ConnectionManager *connection;
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
};

extern ArduinoIoTCloudClass ArduinoCloud;

#endif
