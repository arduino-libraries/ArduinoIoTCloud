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
  #include "tls/utility/ECCX08Cert.h"
#elif defined(BOARD_ESP)
  #include <WiFiClientSecure.h>
#endif

#ifdef BOARD_HAS_OFFLOADED_ECCX08
#include "tls/utility/ECCX08Cert.h"
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

typedef bool (*otaConfirmationStatus)(void);

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

    #if defined(BOARD_HAS_ECCX08) || defined(BOARD_HAS_OFFLOADED_ECCX08)
    int begin(ConnectionHandler & connection, bool const enable_watchdog = true, String brokerAddress = DEFAULT_BROKER_ADDRESS_SECURE_AUTH, uint16_t brokerPort = DEFAULT_BROKER_PORT_SECURE_AUTH);
    #else
    int begin(ConnectionHandler & connection, bool const enable_watchdog = true, String brokerAddress = DEFAULT_BROKER_ADDRESS_USER_PASS_AUTH, uint16_t brokerPort = DEFAULT_BROKER_PORT_USER_PASS_AUTH);
    #endif
    int begin(bool const enable_watchdog = true, String brokerAddress = DEFAULT_BROKER_ADDRESS_SECURE_AUTH, uint16_t brokerPort = DEFAULT_BROKER_PORT_SECURE_AUTH);

    #ifdef BOARD_ESP
    inline void setBoardId        (String const device_id) { setDeviceId(device_id); }
    inline void setSecretDeviceKey(String const password)  { _password = password;  }
    #endif

    inline String   getBrokerAddress() const { return _brokerAddress; }
    inline uint16_t getBrokerPort   () const { return _brokerPort; }

#if OTA_ENABLED

    // The callback is triggered when the OTA is initiated
    // Should return true when the OTA can be applied, false otherwise

/*
    static first_run = true;
    bool sample_ota_confirmation() {
      if (first_run) {
        HMI.show(confirmationModal)
        first_run = false;
      }
      return HMI.getConfirmation();
    }
*/

    void onOTARequestCb(otaConfirmationStatus cb) {
      _get_ota_confirmation = cb;
      _automatic_ota = false;
    }
#endif

  private:
    static const int MQTT_TRANSMIT_BUFFER_SIZE = 256;

    enum class State
    {
      ConnectPhy,
      SyncTime,
      ConnectMqttBroker,
      SubscribeMqttTopics,
      RequestLastValues,
      Connected,
    };

    State _state;

    unsigned long _next_connection_attempt_tick;
    unsigned int _last_connection_attempt_cnt;
    unsigned long _last_sync_request_tick;
    unsigned int _last_sync_request_cnt;
    String _brokerAddress;
    uint16_t _brokerPort;
    uint8_t _mqtt_data_buf[MQTT_TRANSMIT_BUFFER_SIZE];
    int _mqtt_data_len;
    bool _mqtt_data_request_retransmit;

    #if defined(BOARD_HAS_ECCX08)
    ECCX08CertClass _eccx08_cert;
    BearSSLClient _sslClient;
    #elif defined(BOARD_HAS_OFFLOADED_ECCX08)
    ECCX08CertClass _eccx08_cert;
    WiFiBearSSLClient _sslClient;
    #elif defined(BOARD_ESP)
    WiFiClientSecure _sslClient;
    String _password;
    #endif

    MqttClient _mqttClient;

    String _shadowTopicOut;
    String _shadowTopicIn;
    String _dataTopicOut;
    String _dataTopicIn;

#if OTA_ENABLED
    bool _ota_cap;
    int _ota_error;
    String _ota_img_sha256;
    String _ota_url;
    bool _ota_req;
    bool _automatic_ota = true;
#endif /* OTA_ENABLED */

    inline String getTopic_shadowout() { return ( getThingId().length() == 0) ? String("")                            : String("/a/t/" + getThingId() + "/shadow/o"); }
    inline String getTopic_shadowin () { return ( getThingId().length() == 0) ? String("")                            : String("/a/t/" + getThingId() + "/shadow/i"); }
    inline String getTopic_dataout  () { return ( getThingId().length() == 0) ? String("/a/d/" + getDeviceId() + "/e/o") : String("/a/t/" + getThingId() + "/e/o"); }
    inline String getTopic_datain   () { return ( getThingId().length() == 0) ? String("/a/d/" + getDeviceId() + "/e/i") : String("/a/t/" + getThingId() + "/e/i"); }

    State handle_ConnectPhy();
    State handle_SyncTime();
    State handle_ConnectMqttBroker();
    State handle_SubscribeMqttTopics();
    State handle_RequestLastValues();
    State handle_Connected();

    static void onMessage(int length);
    void handleMessage(int length);
    void sendPropertiesToCloud();
    void requestLastValue();
    int write(String const topic, byte const data[], int const length);

#if OTA_ENABLED
    void onOTARequest();
#endif

    otaConfirmationStatus _get_ota_confirmation = {nullptr};
};

/******************************************************************************
 * EXTERN DECLARATION
 ******************************************************************************/

extern ArduinoIoTCloudTCP ArduinoCloud;

#endif