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

    void addPropertyReal(ArduinoCloudProperty& property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(ArduinoCloudProperty& property) = CLOUD_WINS) {
      Permission permission = Permission::ReadWrite;
      if (permission_type == READ) {
        permission = Permission::Read;
      } else if (permission_type == WRITE) {
        permission = Permission::Write;
      } else {
        permission = Permission::ReadWrite;
      }

      if (seconds == ON_CHANGE) {
        Thing.addPropertyReal(property, name, permission).publishOnChange(minDelta, DEFAULT_MIN_TIME_BETWEEN_UPDATES_MILLIS).onUpdate(fn).onSync(synFn);
      } else {
        Thing.addPropertyReal(property, name, permission).publishEvery(seconds).onUpdate(fn).onSync(synFn);
      }
    }
    void addPropertyReal(bool& property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS) {
      ArduinoCloudProperty *p = new CloudWrapperBool(property);
      addPropertyReal(*p, name, permission_type, seconds, fn, minDelta, synFn);
    }
    ArduinoCloudProperty& addPropertyReal(bool& property, String name, Permission const permission) {
      ArduinoCloudProperty *p = new CloudWrapperBool(property);
      return Thing.addPropertyReal(*p, name, permission);
    }
    void addPropertyReal(float& property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS) {
      ArduinoCloudProperty *p = new CloudWrapperFloat(property);
      addPropertyReal(*p, name, permission_type, seconds, fn, minDelta, synFn);
    }
    ArduinoCloudProperty& addPropertyReal(float& property, String name, Permission const permission) {
      ArduinoCloudProperty *p = new CloudWrapperFloat(property);
      return Thing.addPropertyReal(*p, name, permission);
    }
    void addPropertyReal(int& property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS) {
      ArduinoCloudProperty *p = new CloudWrapperInt(property);
      addPropertyReal(*p, name, permission_type, seconds, fn, minDelta, synFn);
    }
    ArduinoCloudProperty& addPropertyReal(int& property, String name, Permission const permission) {
      ArduinoCloudProperty *p = new CloudWrapperInt(property);
      return Thing.addPropertyReal(*p, name, permission);
    }
    void addPropertyReal(String& property, String name, permissionType permission_type = READWRITE, long seconds = ON_CHANGE, void(*fn)(void) = NULL, float minDelta = 0.0f, void(*synFn)(ArduinoCloudProperty & property) = CLOUD_WINS) {
      ArduinoCloudProperty *p = new CloudWrapperString(property);
      addPropertyReal(*p, name, permission_type, seconds, fn, minDelta, synFn);
    }
    ArduinoCloudProperty& addPropertyReal(String& property, String name, Permission const permission) {
      ArduinoCloudProperty *p = new CloudWrapperString(property);
      return Thing.addPropertyReal(*p, name, permission);
    }

    void addCallback(ArduinoIoTCloudEvent const event, OnCloudEventCallback callback) {
		switch (event) {
		case ArduinoIoTCloudEvent::SYNC:       _on_sync_event_callback = callback; break;
		case ArduinoIoTCloudEvent::CONNECT:    _on_connect_event_callback = callback; break;
		case ArduinoIoTCloudEvent::DISCONNECT: _on_disconnect_event_callback = callback; break;
		}
	};

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

	OnCloudEventCallback _on_sync_event_callback = NULL;
	OnCloudEventCallback _on_connect_event_callback = NULL;
	OnCloudEventCallback _on_disconnect_event_callback = NULL;

    static void execCloudEventCallback(OnCloudEventCallback & callback, void * callback_arg) {
		if (callback) {
			(*callback)(callback_arg);
		}
	}
    static void printConnectionStatus(ArduinoIoTConnectionStatus status) {
		switch (status) {
		case ArduinoIoTConnectionStatus::IDLE:         Debug.print(DBG_INFO, "Arduino IoT Cloud Connection status: IDLE");         break;
		case ArduinoIoTConnectionStatus::ERROR:        Debug.print(DBG_ERROR, "Arduino IoT Cloud Connection status: ERROR");        break;
		case ArduinoIoTConnectionStatus::CONNECTING:   Debug.print(DBG_INFO, "Arduino IoT Cloud Connection status: CONNECTING");   break;
		case ArduinoIoTConnectionStatus::RECONNECTING: Debug.print(DBG_INFO, "Arduino IoT Cloud Connection status: RECONNECTING"); break;
		case ArduinoIoTConnectionStatus::CONNECTED:    Debug.print(DBG_INFO, "Arduino IoT Cloud Connection status: CONNECTED");    break;
		case ArduinoIoTConnectionStatus::DISCONNECTED: Debug.print(DBG_ERROR, "Arduino IoT Cloud Connection status: DISCONNECTED"); break;
		}
	}
};

#if defined(ARDUINO_SAMD_MKRGSM1400) || defined(ARDUINO_SAMD_MKRWIFI1010) ||  defined(ARDUINO_SAMD_MKR1000) || defined(ARDUINO_SAMD_NANO_33_IOT)
  //#error HAS_TCP
  #include "ArduinoIoTCloudTCP.h"
  extern ArduinoIoTCloudTCP ArduinoCloud;
#elif defined(ARDUINO_SAMD_MKRWAN1300) || defined(ARDUINO_SAMD_MKRWAN1310)
  //#error HAS_LORA
  #include "ArduinoIoTCloudLPWAN.h"
  //extern ArduinoIoTCloudLPWAN ArduinoCloud;
#endif

#endif
