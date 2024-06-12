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

/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <ArduinoIoTCloud.h>

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

ArduinoIoTCloudClass::ArduinoIoTCloudClass()
: _connection{nullptr}
, _time_service(TimeService)
, _thing_id{"xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"}
, _lib_version{AIOT_CONFIG_LIB_VERSION}
, _device_id{"xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"}
, _cloud_event_callback{nullptr}
{

}

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

void ArduinoIoTCloudClass::push()
{
  requestUpdateForAllProperties(getThingPropertyContainer());
}

bool ArduinoIoTCloudClass::setTimestamp(String const & prop_name, unsigned long const timestamp)
{
  Property * p = getProperty(getThingPropertyContainer(), prop_name);

  if (p == nullptr)
    return false;

  p->setTimestamp(timestamp);

  return true;
}

void ArduinoIoTCloudClass::addCallback(ArduinoIoTCloudEvent const event, OnCloudEventCallback callback)
{
  _cloud_event_callback[static_cast<size_t>(event)] = callback;
}

/* The following methods are used for both LoRa and non-Lora boards */
template <> Property& ArduinoIoTCloudClass::addPropertyReal(bool& property, String name, int tag, Permission const permission)
{
  Property* p = new CloudWrapperBool(property);
  return addPropertyToContainer(getThingPropertyContainer(), *p, name, permission, tag);
}
template <> Property& ArduinoIoTCloudClass::addPropertyReal(float& property, String name, int tag, Permission const permission)
{
  Property* p = new CloudWrapperFloat(property);
  return addPropertyToContainer(getThingPropertyContainer(), *p, name, permission, tag);
}

Property& ArduinoIoTCloudClass::addPropertyReal(String& property, String name, int tag, Permission const permission)
{
  Property* p = new CloudWrapperString(property);
  return addPropertyToContainer(getThingPropertyContainer(), *p, name, permission, tag);
}
Property& ArduinoIoTCloudClass::addPropertyReal(Property& property, String name, int tag, Permission const permission)
{
  return addPropertyToContainer(getThingPropertyContainer(), property, name, permission, tag);
}
Property& ArduinoIoTCloudClass::addPropertyReal(CloudBool& property, String name, int tag, Permission const permission)
{
  return addPropertyToContainer(getThingPropertyContainer(), property, name, permission, tag);
}
Property& ArduinoIoTCloudClass::addPropertyReal(CloudColor& property, String name, int tag, Permission const permission)
{
  return addPropertyToContainer(getThingPropertyContainer(), property, name, permission, tag);
}
Property& ArduinoIoTCloudClass::addPropertyReal(CloudColoredLight& property, String name, int tag, Permission const permission)
{
  return addPropertyToContainer(getThingPropertyContainer(), property, name, permission, tag);
}
Property& ArduinoIoTCloudClass::addPropertyReal(CloudContactSensor& property, String name, int tag, Permission const permission)
{
  return addPropertyToContainer(getThingPropertyContainer(), property, name, permission, tag);
}
Property& ArduinoIoTCloudClass::addPropertyReal(CloudDimmedLight& property, String name, int tag, Permission const permission)
{
  return addPropertyToContainer(getThingPropertyContainer(), property, name, permission, tag);
}
Property& ArduinoIoTCloudClass::addPropertyReal(CloudFloat& property, String name, int tag, Permission const permission)
{
  return addPropertyToContainer(getThingPropertyContainer(), property, name, permission, tag);
}
Property& ArduinoIoTCloudClass::addPropertyReal(CloudLight& property, String name, int tag, Permission const permission)
{
  return addPropertyToContainer(getThingPropertyContainer(), property, name, permission, tag);
}
Property& ArduinoIoTCloudClass::addPropertyReal(CloudLocation& property, String name, int tag, Permission const permission)
{
  return addPropertyToContainer(getThingPropertyContainer(), property, name, permission, tag);
}
Property& ArduinoIoTCloudClass::addPropertyReal(CloudMotionSensor& property, String name, int tag, Permission const permission)
{
  return addPropertyToContainer(getThingPropertyContainer(), property, name, permission, tag);
}
Property& ArduinoIoTCloudClass::addPropertyReal(CloudSchedule& property, String name, int tag, Permission const permission)
{
  return addPropertyToContainer(getThingPropertyContainer(), property, name, permission, tag);
}
Property& ArduinoIoTCloudClass::addPropertyReal(CloudSmartPlug& property, String name, int tag, Permission const permission)
{
  return addPropertyToContainer(getThingPropertyContainer(), property, name, permission, tag);
}
Property& ArduinoIoTCloudClass::addPropertyReal(CloudString& property, String name, int tag, Permission const permission)
{
  return addPropertyToContainer(getThingPropertyContainer(), property, name, permission, tag);
}
Property& ArduinoIoTCloudClass::addPropertyReal(CloudSwitch& property, String name, int tag, Permission const permission)
{
  return addPropertyToContainer(getThingPropertyContainer(), property, name, permission, tag);
}
Property& ArduinoIoTCloudClass::addPropertyReal(CloudTelevision& property, String name, int tag, Permission const permission)
{
  return addPropertyToContainer(getThingPropertyContainer(), property, name, permission, tag);
}
Property& ArduinoIoTCloudClass::addPropertyReal(CloudTemperatureSensor& property, String name, int tag, Permission const permission)
{
  return addPropertyToContainer(getThingPropertyContainer(), property, name, permission, tag);
}

/* The following methods are deprecated but still used for both LoRa and non-LoRa boards */
template <> void ArduinoIoTCloudClass::addPropertyReal(bool& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  Property* p = new CloudWrapperBool(property);
  addPropertyRealInternal(*p, name, tag, permission_type, seconds, fn, minDelta, synFn);
}
template <> void ArduinoIoTCloudClass::addPropertyReal(float& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  Property* p = new CloudWrapperFloat(property);
  addPropertyRealInternal(*p, name, tag, permission_type, seconds, fn, minDelta, synFn);
}

void ArduinoIoTCloudClass::addPropertyReal(String& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  Property* p = new CloudWrapperString(property);
  addPropertyRealInternal(*p, name, tag, permission_type, seconds, fn, minDelta, synFn);
}
void ArduinoIoTCloudClass::addPropertyReal(Property& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  addPropertyRealInternal(property, name, tag, permission_type, seconds, fn, minDelta, synFn);
}
void ArduinoIoTCloudClass::addPropertyReal(CloudBool& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  addPropertyRealInternal(property, name, tag, permission_type, seconds, fn, minDelta, synFn);
}
void ArduinoIoTCloudClass::addPropertyReal(CloudColor& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  addPropertyRealInternal(property, name, tag, permission_type, seconds, fn, minDelta, synFn);
}
void ArduinoIoTCloudClass::addPropertyReal(CloudColoredLight& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  addPropertyRealInternal(property, name, tag, permission_type, seconds, fn, minDelta, synFn);
}
void ArduinoIoTCloudClass::addPropertyReal(CloudContactSensor& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  addPropertyRealInternal(property, name, tag, permission_type, seconds, fn, minDelta, synFn);
}
void ArduinoIoTCloudClass::addPropertyReal(CloudDimmedLight& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  addPropertyRealInternal(property, name, tag, permission_type, seconds, fn, minDelta, synFn);
}
void ArduinoIoTCloudClass::addPropertyReal(CloudFloat& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  addPropertyRealInternal(property, name, tag, permission_type, seconds, fn, minDelta, synFn);
}
void ArduinoIoTCloudClass::addPropertyReal(CloudLight& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  addPropertyRealInternal(property, name, tag, permission_type, seconds, fn, minDelta, synFn);
}
void ArduinoIoTCloudClass::addPropertyReal(CloudLocation& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  addPropertyRealInternal(property, name, tag, permission_type, seconds, fn, minDelta, synFn);
}
void ArduinoIoTCloudClass::addPropertyReal(CloudMotionSensor& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  addPropertyRealInternal(property, name, tag, permission_type, seconds, fn, minDelta, synFn);
}
void ArduinoIoTCloudClass::addPropertyReal(CloudSchedule& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  addPropertyRealInternal(property, name, tag, permission_type, seconds, fn, minDelta, synFn);
}
void ArduinoIoTCloudClass::addPropertyReal(CloudSmartPlug& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  addPropertyRealInternal(property, name, tag, permission_type, seconds, fn, minDelta, synFn);
}
void ArduinoIoTCloudClass::addPropertyReal(CloudString& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  addPropertyRealInternal(property, name, tag, permission_type, seconds, fn, minDelta, synFn);
}
void ArduinoIoTCloudClass::addPropertyReal(CloudSwitch& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  addPropertyRealInternal(property, name, tag, permission_type, seconds, fn, minDelta, synFn);
}
void ArduinoIoTCloudClass::addPropertyReal(CloudTelevision& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  addPropertyRealInternal(property, name, tag, permission_type, seconds, fn, minDelta, synFn);
}
void ArduinoIoTCloudClass::addPropertyReal(CloudTemperatureSensor& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  addPropertyRealInternal(property, name, tag, permission_type, seconds, fn, minDelta, synFn);
}

void ArduinoIoTCloudClass::addPropertyRealInternal(Property& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  Permission permission = Permission::ReadWrite;
  if (permission_type == READ) {
    permission = Permission::Read;
  } else if (permission_type == WRITE) {
    permission = Permission::Write;
  } else {
    permission = Permission::ReadWrite;
  }

  if (seconds == ON_CHANGE) {
    addPropertyToContainer(getThingPropertyContainer(), property, name, permission, tag).publishOnChange(minDelta, Property::DEFAULT_MIN_TIME_BETWEEN_UPDATES_MILLIS).onUpdate(fn).onSync(synFn);
  } else {
    addPropertyToContainer(getThingPropertyContainer(), property, name, permission, tag).publishEvery(seconds).onUpdate(fn).onSync(synFn);
  }
}

/******************************************************************************
 * PROTECTED MEMBER FUNCTIONS
 ******************************************************************************/

void ArduinoIoTCloudClass::execCloudEventCallback(ArduinoIoTCloudEvent const event)
{
  OnCloudEventCallback callback = _cloud_event_callback[static_cast<size_t>(event)];
  if (callback) {
    (*callback)();
  }
}

__attribute__((weak)) void setDebugMessageLevel(int const /* level */)
{
  /* do nothing */
}
