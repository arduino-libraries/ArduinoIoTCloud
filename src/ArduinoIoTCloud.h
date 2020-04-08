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

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <ArduinoIoTCloud_Defines.h>

#include <ArduinoCloudThing.h>
#include <Arduino_ConnectionHandler.h>
#include <Arduino_DebugUtils.h>

#include "types/CloudWrapperBool.h"
#include "types/CloudWrapperFloat.h"
#include "types/CloudWrapperInt.h"
#include "types/CloudWrapperString.h"

#include "CloudSerial.h"

/******************************************************************************
   TYPEDEF
 ******************************************************************************/

typedef enum
{
  READ      = 0x01,
  WRITE     = 0x02,
  READWRITE = READ | WRITE
} permissionType;

enum class ArduinoIoTConnectionStatus
{
  IDLE,
  CONNECTING,
  CONNECTED,
  DISCONNECTED,
  RECONNECTING,
  ERROR,
};

enum class ArduinoIoTSynchronizationStatus
{
  SYNC_STATUS_SYNCHRONIZED,
  SYNC_STATUS_WAIT_FOR_CLOUD_VALUES,
  SYNC_STATUS_VALUES_PROCESSED
};

enum class ArduinoIoTCloudEvent : size_t
{
  SYNC = 0, CONNECT = 1, DISCONNECT = 2
};

typedef void (*OnCloudEventCallback)(void);

/******************************************************************************
 * CLASS DECLARATION
 ******************************************************************************/

class ArduinoIoTCloudClass
{
  public:

    virtual ~ArduinoIoTCloudClass() { }


    virtual int  connect       () = 0;
    virtual bool disconnect    () = 0;
    virtual void update        () = 0;
    virtual int  connected     () = 0;
    virtual void printDebugInfo() = 0;

    inline void     setThingId (String const thing_id)  { _thing_id = thing_id; };
    inline String & getThingId ()                       { return _thing_id; };
    inline void     setDeviveId(String const device_id) { _device_id = device_id; };
    inline String & getDeviceId()                       { return _device_id; };

    inline ConnectionHandler * getConnection()          { return _connection; }

    void addCallback(ArduinoIoTCloudEvent const event, OnCloudEventCallback callback);

#define addProperty( v, ...) addPropertyReal(v, #v, __VA_ARGS__)

    static unsigned long const DEFAULT_MIN_TIME_BETWEEN_UPDATES_MILLIS = 500; /* Data rate throttled to 2 Hz */

    /* The following methods are used for non-LoRa boards which can use the 
     * name of the property to identify a given property within a CBOR message.
     */

    void addPropertyReal(ArduinoCloudProperty& property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS);
    void addPropertyReal(bool& property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS);
    void addPropertyReal(float& property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS);
    void addPropertyReal(int& property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS);
    void addPropertyReal(String& property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS);

    ArduinoCloudProperty& addPropertyReal(bool& property, String name, Permission const permission);
    ArduinoCloudProperty& addPropertyReal(float& property, String name, Permission const permission);
    ArduinoCloudProperty& addPropertyReal(int& property, String name, Permission const permission);
    ArduinoCloudProperty& addPropertyReal(String& property, String name, int tag, Permission const permission);

    /* The following methods are for MKR WAN 1300/1310 LoRa boards since
     * they use a number to identify a given property within a CBOR message.
     * This approach reduces the required amount of data which is of great
     * important when using LoRa.
     */

    void addPropertyReal(ArduinoCloudProperty& property, String name, int tag, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS);
    void addPropertyReal(bool& property, String name, int tag, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS);
    void addPropertyReal(float& property, String name, int tag, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS);
    void addPropertyReal(int& property, String name, int tag, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS);
    void addPropertyReal(String& property, String name, int tag, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS);

    ArduinoCloudProperty& addPropertyReal(bool& property, String name, int tag, Permission const permission);
    ArduinoCloudProperty& addPropertyReal(float& property, String name, int tag, Permission const permission);
    ArduinoCloudProperty& addPropertyReal(int& property, String name, int tag, Permission const permission);
    ArduinoCloudProperty& addPropertyReal(String& property, String name, Permission const permission);

  protected:

    inline ArduinoIoTConnectionStatus getIoTStatus() { return _iot_status; }

    ConnectionHandler * _connection = nullptr;
    ArduinoCloudThing _thing;
    ArduinoIoTConnectionStatus _iot_status = ArduinoIoTConnectionStatus::IDLE;

           NetworkConnectionState checkPhyConnection();
           void execCloudEventCallback(ArduinoIoTCloudEvent const event);
    static void printConnectionStatus(ArduinoIoTConnectionStatus status);

  private:

    String _thing_id = "";
    String _device_id = "";
    OnCloudEventCallback _cloud_event_callback[3] = {nullptr};
};

#ifdef HAS_TCP
  #include "ArduinoIoTCloudTCP.h"
#elif defined(HAS_LORA)
  #include "ArduinoIoTCloudLPWAN.h"
#endif

#endif
