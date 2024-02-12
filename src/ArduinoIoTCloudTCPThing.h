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

#ifndef ARDUINO_IOT_CLOUD_TCP_THING_H
#define ARDUINO_IOT_CLOUD_TCP_THING_H


/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <AIoTC_Config.h>

#include <ArduinoIoTCloud.h>
#include <ArduinoMqttClient.h>

#include "utility/time/TimeService.h"


/******************************************************************************
 * TYPEDEF
 ******************************************************************************/

typedef bool (*onOTARequestCallbackFunc)(void);

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class ArduinoIoTCloudTCPThing
{
  public:

             ArduinoIoTCloudTCPThing();
    virtual ~ArduinoIoTCloudTCPThing() { }


    void update();

    // begin takes an mqtt client
    int begin(MqttClient& mqttClient, TimeServiceClass time_service, PropertyContainer & thing_property_container);
    void updateTimezoneInfo();
    
    inline void     setThingId (String const thing_id)  { _thing_id = thing_id; };
    inline String & getThingId ()                       { return _thing_id; };
    
    inline void     setThingIdOutdatedFlag()            { _thing_id_outdated = true ; }
    inline void     clrThingIdOutdatedFlag()            { _thing_id_outdated = false ; }
    inline bool     getThingIdOutdatedFlag()            { return _thing_id_outdated; }

    inline bool     deviceNotAttached()                 { return _thing_id == ""; }

    inline void     setMqttDataRequestRetransmitFlag()  { _mqtt_data_request_retransmit = true; }
    inline void     clrMqttDataRequestRetransmitFlag()  { _mqtt_data_request_retransmit = false; }

    inline void     setTzOffset(int tz_offset)  { _tz_offset = tz_offset; }
    inline int &     getTzOffset()         { return _tz_offset; }

    inline void setTzDstUntil(unsigned int tz_dst_until) { _tz_dst_until = tz_dst_until; }
    inline unsigned int &  getTzDstUntil() { return _tz_dst_until; }
    
  private:
    static const int MQTT_TRANSMIT_BUFFER_SIZE = 256;

    enum class State
    {
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

    MqttClient &_mqttClient;
    PropertyContainer & _thing_property_container;
    TimeServiceClass &_time_service;

    String _thing_id;
    bool _thing_id_outdated;
    bool _deviceSubscribedToThing;
    unsigned int _last_checked_property_index;

    int _tz_offset;
    unsigned int _tz_dst_until;

    String _shadowTopicOut;
    String _shadowTopicIn;
    String _dataTopicOut;
    String _dataTopicIn;

    inline String getTopic_shadowout() { return ( getThingId().length() == 0) ? String("") : String("/a/t/" + getThingId() + "/shadow/o"); }
    inline String getTopic_shadowin () { return ( getThingId().length() == 0) ? String("") : String("/a/t/" + getThingId() + "/shadow/i"); }
    inline String getTopic_dataout  () { return ( getThingId().length() == 0) ? String("") : String("/a/t/" + getThingId() + "/e/o"); }
    inline String getTopic_datain   () { return ( getThingId().length() == 0) ? String("") : String("/a/t/" + getThingId() + "/e/i"); }

    State handle_WaitDeviceConfig();
    State handle_CheckDeviceConfig();
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

    void updateThingTopics();
};

#endif
