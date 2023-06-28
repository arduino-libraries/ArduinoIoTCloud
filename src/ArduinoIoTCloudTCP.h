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

#include <AIoTC_Config.h>

#include <ArduinoIoTCloud.h>

#ifdef BOARD_HAS_ECCX08
  #include "tls/BearSSLClient.h"
  #include "tls/utility/CryptoUtil.h"
#elif defined(BOARD_ESP)
  #include <WiFiClientSecure.h>
#elif defined(ARDUINO_UNOR4_WIFI)
  #include <WiFiSSLClient.h>
#elif defined(ARDUINO_PORTENTA_C33)
  #include "tls/utility/CryptoUtil.h"
  #include <SSLClient.h>
#elif defined(BOARD_HAS_SE050)
  #include "tls/utility/CryptoUtil.h"
  #include <WiFiSSLSE050Client.h>
#endif

#ifdef BOARD_HAS_OFFLOADED_ECCX08
#include "tls/utility/CryptoUtil.h"
#include <WiFiSSLClient.h>
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
 * TYPEDEF
 ******************************************************************************/

typedef bool (*onOTARequestCallbackFunc)(void);

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class ArduinoIoTCloudTCP: public ArduinoIoTCloudClass
{
  public:

             ArduinoIoTCloudTCP();
    virtual ~ArduinoIoTCloudTCP() { }


    virtual void update        () override;
    virtual int  connected     () override;
    virtual void printDebugInfo() override;

    #if defined(BOARD_HAS_ECCX08) || defined(BOARD_HAS_OFFLOADED_ECCX08) || defined(BOARD_HAS_SE050)
    int begin(ConnectionHandler & connection, bool const enable_watchdog = true, String brokerAddress = DEFAULT_BROKER_ADDRESS_SECURE_AUTH, uint16_t brokerPort = DEFAULT_BROKER_PORT_SECURE_AUTH);
    #else
    int begin(ConnectionHandler & connection, bool const enable_watchdog = true, String brokerAddress = DEFAULT_BROKER_ADDRESS_USER_PASS_AUTH, uint16_t brokerPort = DEFAULT_BROKER_PORT_USER_PASS_AUTH);
    #endif
    int begin(bool const enable_watchdog = true, String brokerAddress = DEFAULT_BROKER_ADDRESS_SECURE_AUTH, uint16_t brokerPort = DEFAULT_BROKER_PORT_SECURE_AUTH);

    #ifdef BOARD_HAS_SECRET_KEY
    inline void setBoardId        (String const device_id) { setDeviceId(device_id); }
    inline void setSecretDeviceKey(String const password)  { _password = password;  }
    #endif

    inline String   getBrokerAddress() const { return _brokerAddress; }
    inline uint16_t getBrokerPort   () const { return _brokerPort; }

#if OTA_ENABLED
    /* The callback is triggered when the OTA is initiated and it gets executed until _ota_req flag is cleared.
     * It should return true when the OTA can be applied or false otherwise.
     * See example ArduinoIoTCloud-DeferredOTA.ino
     */
    void onOTARequestCb(onOTARequestCallbackFunc cb) {
      _get_ota_confirmation = cb;
      _ask_user_before_executing_ota = true;
    }
#endif

  private:
    static const int MQTT_TRANSMIT_BUFFER_SIZE = 256;

    enum class State
    {
      ConnectPhy,
      SyncTime,
      ConnectMqttBroker,
      SendDeviceProperties,
      SubscribeDeviceTopic,
      WaitDeviceConfig,
      CheckDeviceConfig,
      SubscribeThingTopics,
      RequestLastValues,
      Connected,
      Disconnect,
    };

    State _state;

    unsigned long _next_connection_attempt_tick;
    unsigned int _last_connection_attempt_cnt;
    unsigned long _next_device_subscribe_attempt_tick;
    unsigned int _last_device_subscribe_cnt;
    unsigned int _last_device_attach_cnt;
    unsigned long _last_sync_request_tick;
    unsigned int _last_sync_request_cnt;
    unsigned long _last_subscribe_request_tick;
    unsigned int  _last_subscribe_request_cnt;
    String _brokerAddress;
    uint16_t _brokerPort;
    uint8_t _mqtt_data_buf[MQTT_TRANSMIT_BUFFER_SIZE];
    int _mqtt_data_len;
    bool _mqtt_data_request_retransmit;

    #if defined(BOARD_HAS_ECCX08)
    ArduinoIoTCloudCertClass _cert;
    BearSSLClient _sslClient;
    CryptoUtil _crypto;
    #elif defined(BOARD_HAS_OFFLOADED_ECCX08)
    ArduinoIoTCloudCertClass _cert;
    WiFiBearSSLClient _sslClient;
    CryptoUtil _crypto;
    #elif defined(BOARD_ESP)
    WiFiClientSecure _sslClient;
    #elif defined(ARDUINO_UNOR4_WIFI)
    WiFiSSLClient _sslClient;
    #elif defined(ARDUINO_PORTENTA_C33)
    ArduinoIoTCloudCertClass _cert;
    SSLClient _sslClient;
    CryptoUtil _crypto;
    #elif defined(BOARD_HAS_SE050)
    ArduinoIoTCloudCertClass _cert;
    WiFiSSLSE050Client _sslClient;
    CryptoUtil _crypto;
    #endif

    #if defined (BOARD_HAS_SECRET_KEY)
    String _password;
    #endif

    MqttClient _mqttClient;

    String _deviceTopicOut;
    String _deviceTopicIn;
    String _shadowTopicOut;
    String _shadowTopicIn;
    String _dataTopicOut;
    String _dataTopicIn;

    bool _deviceSubscribedToThing;

#if OTA_ENABLED
    bool _ota_cap;
    int _ota_error;
    String _ota_img_sha256;
    String _ota_url;
    bool _ota_req;
    bool _ask_user_before_executing_ota;
    onOTARequestCallbackFunc _get_ota_confirmation;
#endif /* OTA_ENABLED */

    inline String getTopic_deviceout() { return String("/a/d/" + getDeviceId() + "/e/o");}
    inline String getTopic_devicein () { return String("/a/d/" + getDeviceId() + "/e/i");}
    inline String getTopic_shadowout() { return ( getThingId().length() == 0) ? String("") : String("/a/t/" + getThingId() + "/shadow/o"); }
    inline String getTopic_shadowin () { return ( getThingId().length() == 0) ? String("") : String("/a/t/" + getThingId() + "/shadow/i"); }
    inline String getTopic_dataout  () { return ( getThingId().length() == 0) ? String("") : String("/a/t/" + getThingId() + "/e/o"); }
    inline String getTopic_datain   () { return ( getThingId().length() == 0) ? String("") : String("/a/t/" + getThingId() + "/e/i"); }

    State handle_ConnectPhy();
    State handle_SyncTime();
    State handle_ConnectMqttBroker();
    State handle_SendDeviceProperties();
    State handle_WaitDeviceConfig();
    State handle_CheckDeviceConfig();
    State handle_SubscribeDeviceTopic();
    State handle_SubscribeThingTopics();
    State handle_RequestLastValues();
    State handle_Connected();
    State handle_Disconnect();

    static void onMessage(int length);
    void handleMessage(int length);
    void sendPropertyContainerToCloud(String const topic, PropertyContainer & property_container, unsigned int & current_property_index);
    void sendThingPropertiesToCloud();
    void sendDevicePropertiesToCloud();
    void requestLastValue();
    int write(String const topic, byte const data[], int const length);

#if OTA_ENABLED
    void sendDevicePropertyToCloud(String const name);
#endif

    void updateThingTopics();
};

/******************************************************************************
 * EXTERN DECLARATION
 ******************************************************************************/

extern ArduinoIoTCloudTCP ArduinoCloud;

#endif
