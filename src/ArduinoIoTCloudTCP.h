/*
  This file is part of the ArduinoIoTCloud library.

  Copyright (c) 2019 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef ARDUINO_IOT_CLOUD_TCP_H
#define ARDUINO_IOT_CLOUD_TCP_H

/******************************************************************************
  INCLUDE
 ******************************************************************************/

#include <AIoTC_Config.h>
#include <ArduinoIoTCloud.h>
#include <ArduinoMqttClient.h>
#include <ArduinoIoTCloudThing.h>
#include <ArduinoIoTCloudDevice.h>

#if defined(BOARD_HAS_SECURE_ELEMENT)
  #include <Arduino_SecureElement.h>
  #include <utility/SElementArduinoCloudDeviceId.h>
  #include <utility/SElementArduinoCloudCertificate.h>
#endif

#include <tls/utility/TLSClientMqtt.h>
#include <tls/utility/TLSClientOta.h>

#if OTA_ENABLED
  #include <ota/OTA.h>
#endif

#include "cbor/IoTCloudMessageDecoder.h"
#include "cbor/IoTCloudMessageEncoder.h"

/******************************************************************************
  CONSTANTS
 ******************************************************************************/
static constexpr char DEFAULT_BROKER_ADDRESS[] = "iot.arduino.cc";
static constexpr uint16_t DEFAULT_BROKER_PORT_SECURE_AUTH = 8885;
static constexpr uint16_t DEFAULT_BROKER_PORT_USER_PASS_AUTH = 8884;
static constexpr uint16_t DEFAULT_BROKER_PORT_AUTO = 0;

/******************************************************************************
  TYPEDEF
 ******************************************************************************/

typedef bool (*onOTARequestCallbackFunc)(void);

/******************************************************************************
  CLASS DECLARATION
 ******************************************************************************/

class ArduinoIoTCloudTCP: public ArduinoIoTCloudClass
{
  public:

             ArduinoIoTCloudTCP();
    virtual ~ArduinoIoTCloudTCP() { }

    virtual void update        () override;
    virtual int  connected     () override;
    virtual void printDebugInfo() override;
    virtual void disconnect    () override;

    int begin(ConnectionHandler & connection, bool const enable_watchdog = true, String brokerAddress = DEFAULT_BROKER_ADDRESS, uint16_t brokerPort = DEFAULT_BROKER_PORT_AUTO, bool auto_reconnect = true);
    int begin(bool const enable_watchdog = true, String brokerAddress = DEFAULT_BROKER_ADDRESS, uint16_t brokerPort = DEFAULT_BROKER_PORT_AUTO, bool auto_reconnect = true);

#if defined(BOARD_HAS_SECURE_ELEMENT)
    int updateCertificate(String authorityKeyIdentifier, String serialNumber, String notBefore, String notAfter, String signature);
#endif

#ifdef BOARD_HAS_SECRET_KEY
    inline void setBoardId        (String const device_id) { setDeviceId(device_id); }
    inline void setSecretDeviceKey(String const password)  { _password = password;  }
#endif

    inline String   getBrokerAddress() const { return _brokerAddress; }
    inline uint16_t getBrokerPort   () const { return _brokerPort; }

    inline PropertyContainer &getThingPropertyContainer() { return _thing.getPropertyContainer(); }

#if OTA_ENABLED
    /* The callback is triggered when the OTA is initiated and it gets executed until _ota_req flag is cleared.
     * It should return true when the OTA can be applied or false otherwise.
     * See example ArduinoIoTCloud-DeferredOTA.ino
     */
    void onOTARequestCb(onOTARequestCallbackFunc cb) {
      _get_ota_confirmation = cb;

      if(_get_ota_confirmation) {
        _ota.enableOtaPolicy(OTACloudProcessInterface::ApprovalRequired);
      } else {
        _ota.disableOtaPolicy(OTACloudProcessInterface::ApprovalRequired);
      }
    }

    /* Slower but more reliable in some corner cases */
    void setOTAChunkMode(bool enable = true) {
      if(enable) {
        _ota.enableOtaPolicy(OTACloudProcessInterface::ChunkDownload);
      } else {
        _ota.disableOtaPolicy(OTACloudProcessInterface::ChunkDownload);
      }
    }
#endif

  private:
    static const int MQTT_TRANSMIT_BUFFER_SIZE = 256;

    enum class State
    {
      ConfigPhy,
      UpdatePhy,
      Init,
      ConnectPhy,
      SyncTime,
      ConnectMqttBroker,
      Connected,
      Disconnect,
      Disconnected,
    };

    State _state;
    TimedAttempt _connection_attempt;
    MessageStream _message_stream;
    ArduinoCloudThing _thing;
    ArduinoCloudDevice _device;

    ArduinoIoTAuthenticationMode _authMode;
    String _brokerAddress;
    uint16_t _brokerPort;
    uint8_t _mqtt_data_buf[MQTT_TRANSMIT_BUFFER_SIZE];
    int _mqtt_data_len;
    bool _mqtt_data_request_retransmit;
    bool _enable_watchdog;
    bool _auto_reconnect;

#if defined(BOARD_HAS_SECRET_KEY)
    String _password;
#endif

#if defined(BOARD_HAS_SECURE_ELEMENT)
    SecureElement _selement;
    ECP256Certificate _cert;
    /* Flag used to store updated device certificate after broker connection has succeeded */
    bool _writeCertOnConnect;
#endif

    TLSClientMqtt _brokerClient;
    MqttClient _mqttClient;

    String _messageTopicOut;
    String _messageTopicIn;
    String _dataTopicOut;
    String _dataTopicIn;

#if OTA_ENABLED
    TLSClientOta _otaClient;
    ArduinoCloudOTA _ota;
    onOTARequestCallbackFunc _get_ota_confirmation;
#endif /* OTA_ENABLED */

    inline String getTopic_messageout() { return String("/a/d/" + getDeviceId() + "/c/up");}
    inline String getTopic_messagein () { return String("/a/d/" + getDeviceId() + "/c/dw");}

    inline String getTopic_dataout  () { return ( getThingId().length() == 0) ? String("") : String("/a/t/" + getThingId() + "/e/o"); }
    inline String getTopic_datain   () { return ( getThingId().length() == 0) ? String("") : String("/a/t/" + getThingId() + "/e/i"); }

    State handle_ConfigPhy();
    State handle_UpdatePhy();
    State handle_Init();
    State handle_ConnectPhy();
    State handle_SyncTime();
    State handle_ConnectMqttBroker();
    State handle_Connected();
    State handle_Disconnect();

    static void onMessage(int length);
    void handleMessage(int length);
    void sendMessage(Message * msg);
    void sendPropertyContainerToCloud(String const topic, PropertyContainer & property_container, unsigned int & current_property_index);

    void attachThing(String thingId);
    void detachThing();
    int write(String const topic, byte const data[], int const length);

};

/******************************************************************************
  EXTERN DECLARATION
 ******************************************************************************/

extern ArduinoIoTCloudTCP ArduinoCloud;

#endif
