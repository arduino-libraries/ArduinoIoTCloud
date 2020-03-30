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

#ifndef ARDUINO_IOT_CLOUD_TCP_H
#define ARDUINO_IOT_CLOUD_TCP_H

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

#ifdef BOARD_HAS_ECCX08
  #include <ArduinoBearSSL.h>
  #include "utility/crypto/CryptoUtil.h"
#elif defined(BOARD_ESP)
  #include <WiFiClientSecure.h>
#endif

#include <ArduinoMqttClient.h>

/******************************************************************************
   CONSTANTS
 ******************************************************************************/

static char const DEFAULT_BROKER_ADDRESS_SECURE_AUTH[] = "mqtts-sa.iot.arduino.cc";
static uint16_t const DEFAULT_BROKER_PORT_SECURE_AUTH = 8883;
static char const DEFAULT_BROKER_ADDRESS_USER_PASS_AUTH[] = "mqtts-up.iot.arduino.cc";
static uint16_t const DEFAULT_BROKER_PORT_USER_PASS_AUTH = 8884;

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class ArduinoIoTCloudTCP: public ArduinoIoTCloudClass {
  public:

    static const int TIMEOUT_FOR_LASTVALUES_SYNC = 10000;

    ArduinoIoTCloudTCP();
    ~ArduinoIoTCloudTCP();
    int connect();
    bool disconnect();
    int connected();
    void update();
    void printDebugInfo();
    #ifdef BOARD_HAS_ECCX08
    int begin(ConnectionHandler & connection, String brokerAddress = DEFAULT_BROKER_ADDRESS_SECURE_AUTH, uint16_t brokerPort = DEFAULT_BROKER_PORT_SECURE_AUTH);
    #else
    int begin(ConnectionHandler & connection, String brokerAddress = DEFAULT_BROKER_ADDRESS_USER_PASS_AUTH, uint16_t brokerPort = DEFAULT_BROKER_PORT_USER_PASS_AUTH);
    #endif
    int begin(String brokerAddress = DEFAULT_BROKER_ADDRESS_SECURE_AUTH, uint16_t brokerPort = DEFAULT_BROKER_PORT_SECURE_AUTH);
    // Class constant declaration
    static const int MQTT_TRANSMIT_BUFFER_SIZE = 256;

    #ifdef BOARD_ESP
    inline void setBoardId(String const device_id) {
      _device_id = device_id;
    }
    inline void setSecretDeviceKey(String const password) {
      _password = password;
    }
    #endif

    inline ConnectionHandler * getConnection() {
      return _connection;
    }

    String getBrokerAddress() {
      return _brokerAddress;
    }
    uint16_t getBrokerPort() {
      return _brokerPort;
    }

    // Clean up existing Mqtt connection, create a new one and initialize it
    int reconnect();

  protected:
    friend class CloudSerialClass;
    // Used to initialize MQTTClient
    void mqttClientBegin();
    // Function in charge of perform MQTT reconnection, basing on class parameters(retries,and timeout)
    bool mqttReconnect(int const maxRetries, int const timeout);
    // Used to retrieve last values from _shadowTopicIn
    int writeStdout(const byte data[], int length);
    int writeProperties(const byte data[], int length);
    int writeShadowOut(const byte data[], int length);

    void requestLastValue();

  private:
    ConnectionHandler * _connection;
    String _brokerAddress;
    uint16_t _brokerPort;
    uint8_t _mqtt_data_buf[MQTT_TRANSMIT_BUFFER_SIZE];
    int _mqtt_data_len;
    bool _mqtt_data_request_retransmit;

    #ifdef BOARD_HAS_ECCX08
    ECCX08CertClass _eccx08_cert;
    BearSSLClient* _sslClient;
    #elif defined(BOARD_ESP)
    WiFiClientSecure* _sslClient;
    String _password;
    #endif

    MqttClient* _mqttClient;

    // Class attribute to define MTTQ topics 2 for stdIn/out and 2 for data, in order to avoid getting previous pupblished payload
    String _stdinTopic;
    String _stdoutTopic;
    String _shadowTopicOut;
    String _shadowTopicIn;
    String _dataTopicOut;
    String _dataTopicIn;
    String _otaTopic;

    static void onMessage(int length);

    void handleMessage(int length);

    void sendPropertiesToCloud();
    NetworkConnectionState checkPhyConnection();
    ArduinoIoTConnectionStatus checkCloudConnection();
};

/******************************************************************************
 * EXTERN DECLARATION
 ******************************************************************************/

extern ArduinoIoTCloudTCP ArduinoCloud;

#endif