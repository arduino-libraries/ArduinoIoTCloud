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

#include <ArduinoIoTCloud_Defines.h>



#include <ArduinoCloudThing.h>

#include <Arduino_DebugUtils.h>
#include "types/CloudWrapperBool.h"
#include "types/CloudWrapperFloat.h"
#include "types/CloudWrapperInt.h"
#include "types/CloudWrapperString.h"


#include "CloudSerial.h"


typedef enum {
  READ      = 0x01,
  WRITE     = 0x02,
  READWRITE = READ | WRITE
} permissionType;


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

/*************************************************
		Pure Virtual Class Definition
**************************************************/
class ArduinoIoTCloudClass {

  public:
    static const int TIMEOUT_FOR_LASTVALUES_SYNC = 10000;
    /*Public Virtual Functions*/
    virtual int  connect() = 0;
    virtual bool disconnect() = 0;

    virtual void update() = 0;
    virtual void update(int const reconnectionMaxRetries, int const reconnectionTimeoutMs) __attribute__((deprecated)) = 0;
    virtual void update(CallbackFunc onSyncCompleteCallback) __attribute__((deprecated)) = 0; /* Attention: Function is deprecated - use 'addCallback(ArduinoIoTCloudConnectionEvent::SYNC, &onSync)' for adding a onSyncCallback instead */

    virtual int connected() = 0;

    virtual void connectionCheck() = 0;

    virtual void printDebugInfo() = 0;

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

    void addPropertyReal(ArduinoCloudProperty& property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS);

    void addPropertyReal(ArduinoCloudProperty& property, String name, int tag, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS);

    void addPropertyReal(bool& property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS);
    void addPropertyReal(bool& property, String name, int tag, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS);
    ArduinoCloudProperty& addPropertyReal(bool& property, String name, Permission const permission);
    ArduinoCloudProperty& addPropertyReal(bool& property, String name, int tag, Permission const permission);

    void addPropertyReal(float& property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS);

    void addPropertyReal(float& property, String name, int tag, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS);

    ArduinoCloudProperty& addPropertyReal(float& property, String name, Permission const permission);

    ArduinoCloudProperty& addPropertyReal(float& property, String name, int tag, Permission const permission);

    void addPropertyReal(int& property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS);

    void addPropertyReal(int& property, String name, int tag, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS);

    ArduinoCloudProperty& addPropertyReal(int& property, String name, Permission const permission);

    ArduinoCloudProperty& addPropertyReal(int& property, String name, int tag, Permission const permission);

    void addPropertyReal(String& property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS);
    void addPropertyReal(String& property, String name, int tag, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS);
    ArduinoCloudProperty& addPropertyReal(String& property, String name, Permission const permission);

    ArduinoCloudProperty& addPropertyReal(String& property, String name, int tag, Permission const permission);

    void addCallback(ArduinoIoTCloudEvent const event, OnCloudEventCallback callback);


  protected:

    virtual int writeStdout(const byte data[], int length) = 0;
    virtual int writeProperties(const byte data[], int length) = 0;
    virtual int writeShadowOut(const byte data[], int length) = 0;




    ArduinoIoTConnectionStatus getIoTStatus() {
      return _iotStatus;
    }

    ArduinoIoTConnectionStatus _iotStatus = ArduinoIoTConnectionStatus::IDLE;



    ArduinoIoTSynchronizationStatus _syncStatus = ArduinoIoTSynchronizationStatus::SYNC_STATUS_SYNCHRONIZED;


    String _device_id = "";
    String _thing_id = "";


    ArduinoCloudThing Thing;

    int _lastSyncRequestTickTime = 0;

    OnCloudEventCallback _on_sync_event_callback       = NULL;
    OnCloudEventCallback _on_connect_event_callback    = NULL;
    OnCloudEventCallback _on_disconnect_event_callback = NULL;

    static void execCloudEventCallback(OnCloudEventCallback & callback, void * callback_arg);
    static void printConnectionStatus(ArduinoIoTConnectionStatus status);
};

#ifdef HAS_TCP
  #include "ArduinoIoTCloudTCP.h"

#elif defined(HAS_LORA)
  #include "ArduinoIoTCloudLPWAN.h"

#endif

#endif
