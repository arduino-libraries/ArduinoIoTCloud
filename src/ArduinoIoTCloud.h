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

#include <AIoTC_Config.h>

#include <Arduino_ConnectionHandler.h>

#if defined(DEBUG_ERROR) || defined(DEBUG_WARNING) || defined(DEBUG_INFO) || defined(DEBUG_DEBUG) || defined(DEBUG_VERBOSE)
#  include <Arduino_DebugUtils.h>
#endif

#include "AIoTC_Const.h"

#include "cbor/CBORDecoder.h"

#include "property/Property.h"
#include "property/PropertyContainer.h"
#include "property/types/CloudWrapperBool.h"
#include "property/types/CloudWrapperFloat.h"
#include "property/types/CloudWrapperInt.h"
#include "property/types/CloudWrapperUnsignedInt.h"
#include "property/types/CloudWrapperString.h"

#include "utility/time/TimeService.h"

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

             ArduinoIoTCloudClass();
    virtual ~ArduinoIoTCloudClass() { }


    virtual void update        () = 0;
    virtual int  connected     () = 0;
    virtual void printDebugInfo() = 0;

            void push();
            bool setTimestamp(String const & prop_name, unsigned long const timestamp);

    inline void     setThingId (String const thing_id)  { _thing_id = thing_id; };
    inline String & getThingId ()                       { return _thing_id; };
    inline void     setDeviceId(String const device_id) { _device_id = device_id; };
    inline String & getDeviceId()                       { return _device_id; };

    inline void     setThingIdOutdatedFlag()            { _thing_id_outdated = true ; }
    inline void     clrThingIdOutdatedFlag()            { _thing_id_outdated = false ; }
    inline bool     getThingIdOutdatedFlag()            { return _thing_id_outdated; }

    inline bool     deviceNotAttached()                 { return _thing_id == ""; }

    inline ConnectionHandler * getConnection()          { return _connection; }

    inline unsigned long getInternalTime()              { return _time_service.getTime(); }
    inline unsigned long getLocalTime()                 { return _time_service.getLocalTime(); }
    inline void          updateInternalTimezoneInfo()   { _time_service.setTimeZoneData(_tz_offset, _tz_dst_until); }

    void addCallback(ArduinoIoTCloudEvent const event, OnCloudEventCallback callback);

#define addProperty( v, ...) addPropertyReal(v, #v, __VA_ARGS__)

    /* The following methods are used for non-LoRa boards which can use the 
     * name of the property to identify a given property within a CBOR message.
     */

    void addPropertyReal(Property& property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(Property & property) = CLOUD_WINS) __attribute__((deprecated("Use addProperty(property, Permission::ReadWrite) instead.")));
    void addPropertyReal(bool& property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(Property & property) = CLOUD_WINS) __attribute__((deprecated("Use addProperty(property, Permission::ReadWrite) instead.")));
    void addPropertyReal(float& property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(Property & property) = CLOUD_WINS) __attribute__((deprecated("Use addProperty(property, Permission::ReadWrite) instead.")));
    void addPropertyReal(int& property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0, void(*synFn)(Property & property) = CLOUD_WINS) __attribute__((deprecated("Use addProperty(property, Permission::ReadWrite) instead.")));
    void addPropertyReal(unsigned int& property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0, void(*synFn)(Property & property) = CLOUD_WINS) __attribute__((deprecated("Use addProperty(property, Permission::ReadWrite) instead.")));
    void addPropertyReal(String& property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(Property & property) = CLOUD_WINS) __attribute__((deprecated("Use addProperty(property, Permission::ReadWrite) instead.")));

    Property& addPropertyReal(Property& property, String name, Permission const permission);
    Property& addPropertyReal(bool& property, String name, Permission const permission);
    Property& addPropertyReal(float& property, String name, Permission const permission);
    Property& addPropertyReal(int& property, String name, Permission const permission);
    Property& addPropertyReal(unsigned int& property, String name, Permission const permission);
    Property& addPropertyReal(String& property, String name, Permission const permission);

    Property& addPropertyReal(Property& property, PropertyContainer &prop_cont, String name, int tag, Permission const permission);
    Property& addPropertyReal(bool& property, PropertyContainer &prop_cont, String name, int tag, Permission const permission);
    Property& addPropertyReal(float& property, PropertyContainer &prop_cont, String name, int tag, Permission const permission);
    Property& addPropertyReal(int& property, PropertyContainer &prop_cont, String name, int tag, Permission const permission);
    Property& addPropertyReal(unsigned int& property, PropertyContainer &prop_cont, String name, int tag, Permission const permission);
    Property& addPropertyReal(String& property, PropertyContainer &prop_cont, String name, int tag, Permission const permission);

    Property& addPropertyReal(Property& property, PropertyContainer &prop_cont, String name, Permission const permission);
    Property& addPropertyReal(bool& property, PropertyContainer &prop_cont, String name, Permission const permission);
    Property& addPropertyReal(float& property, PropertyContainer &prop_cont, String name, Permission const permission);
    Property& addPropertyReal(int& property, PropertyContainer &prop_cont, String name, Permission const permission);
    Property& addPropertyReal(unsigned int& property, PropertyContainer &prop_cont, String name, Permission const permission);
    Property& addPropertyReal(String& property, PropertyContainer &prop_cont, String name, Permission const permission);

    /* The following methods are for MKR WAN 1300/1310 LoRa boards since
     * they use a number to identify a given property within a CBOR message.
     * This approach reduces the required amount of data which is of great
     * important when using LoRa.
     */

    void addPropertyReal(Property& property, String name, int tag, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(Property & property) = CLOUD_WINS) __attribute__((deprecated("Use addProperty(property, Permission::ReadWrite) instead.")));
    void addPropertyReal(bool& property, String name, int tag, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(Property & property) = CLOUD_WINS) __attribute__((deprecated("Use addProperty(property, Permission::ReadWrite) instead.")));
    void addPropertyReal(float& property, String name, int tag, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(Property & property) = CLOUD_WINS) __attribute__((deprecated("Use addProperty(property, Permission::ReadWrite) instead.")));
    void addPropertyReal(int& property, String name, int tag, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0, void(*synFn)(Property & property) = CLOUD_WINS) __attribute__((deprecated("Use addProperty(property, Permission::ReadWrite) instead.")));
    void addPropertyReal(unsigned int& property, String name, int tag, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0, void(*synFn)(Property & property) = CLOUD_WINS) __attribute__((deprecated("Use addProperty(property, Permission::ReadWrite) instead.")));
    void addPropertyReal(String& property, String name, int tag, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(Property & property) = CLOUD_WINS) __attribute__((deprecated("Use addProperty(property, Permission::ReadWrite) instead.")));

    Property& addPropertyReal(Property& property, String name, int tag, Permission const permission);
    Property& addPropertyReal(bool& property, String name, int tag, Permission const permission);
    Property& addPropertyReal(float& property, String name, int tag, Permission const permission);
    Property& addPropertyReal(int& property, String name, int tag, Permission const permission);
    Property& addPropertyReal(unsigned int& property, String name, int tag, Permission const permission);
    Property& addPropertyReal(String& property, String name, int tag, Permission const permission);

  protected:

    ConnectionHandler * _connection;
    PropertyContainer _device_property_container;
    PropertyContainer _thing_property_container;
    unsigned int _last_checked_property_index;
    TimeService & _time_service;
    int _tz_offset;
    unsigned int _tz_dst_until;
    String _thing_id;
    String _lib_version;

    void execCloudEventCallback(ArduinoIoTCloudEvent const event);

  private:

    String _device_id;
    OnCloudEventCallback _cloud_event_callback[3];
    bool _thing_id_outdated;
};

#ifdef HAS_TCP
  #include "ArduinoIoTCloudTCP.h"
#elif defined(HAS_LORA)
  #include "ArduinoIoTCloudLPWAN.h"
#endif

// declaration for boards without debug library
void setDebugMessageLevel(int const level);

#endif
