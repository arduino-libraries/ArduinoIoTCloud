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
#include <AIoTC_Types.h>
#include <ArduinoMqttClient.h>

#include "utility/time/TimeService.h"
#include "property/PropertyContainer.h"
#include "cbor/CBOREncoder.h"
#include "cbor/CBORDecoder.h"

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class ArduinoIoTCloudTCPThing
{
  public:

             ArduinoIoTCloudTCPThing();
    virtual ~ArduinoIoTCloudTCPThing() { }


    void update();
    void handleMessage(String topic,uint8_t const * const bytes, int length);

    // begin takes an mqtt client
    int begin(MqttClient *mqttClient, TimeServiceClass *time_service, ExecCloudEventCallback event_callback);
    int connected();
    void updateTimezoneInfo();

    Property& addPropertyReal(Property& property, String name, int tag, Permission const permission, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(Property & property) = CLOUD_WINS);
    
    void push();
    bool setTimestamp(String const & prop_name, unsigned long const timestamp);

    inline void     setThingId (String const thing_id)  { _thing_id = thing_id; };
    inline String & getThingId ()                       { return _thing_id; };
    
    inline void     setThingIdOutdatedFlag()            { _thing_id_outdated = true ; }
    inline void     clrThingIdOutdatedFlag()            { _thing_id_outdated = false ; }
    inline bool     getThingIdOutdatedFlag()            { return _thing_id_outdated; }

    inline bool     deviceNotAttached()                 { return _thing_id == ""; }

    inline void     setMqttDataRequestRetransmitFlag()  { _mqtt_data_request_retransmit = true; }
    inline void     clrMqttDataRequestRetransmitFlag()  { _mqtt_data_request_retransmit = false; }

    inline void setLastValueReceived() {_last_values_received = true;}
    inline void clrLastValueReceived() {_last_values_received = false;}
    
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

    unsigned long _last_sync_request_tick;
    unsigned int _last_sync_request_cnt;
    unsigned long _last_subscribe_request_tick;
    unsigned int  _last_subscribe_request_cnt;
    String _brokerAddress;
    uint16_t _brokerPort;
    uint8_t _mqtt_data_buf[MQTT_TRANSMIT_BUFFER_SIZE];
    int _mqtt_data_len;
    bool _mqtt_data_request_retransmit;

    MqttClient *_mqttClient;
    TimeServiceClass *_time_service;
    ExecCloudEventCallback _event_callback;

    String _thing_id;
    bool _thing_id_outdated;
    bool _deviceSubscribedToThing;
    unsigned int _last_checked_property_index;
    bool _last_values_received;

    String _shadowTopicOut;
    String _shadowTopicIn;
    String _dataTopicOut;
    String _dataTopicIn;

    PropertyContainer _thing_property_container;

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

    void sendPropertyContainerToCloud(String const topic, PropertyContainer & property_container, unsigned int & current_property_index);
    void sendThingPropertiesToCloud();
    void requestLastValue();
    int write(String const topic, byte const data[], int const length);

    void updateThingTopics();
};

#endif
