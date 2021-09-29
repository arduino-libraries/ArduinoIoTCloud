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
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

void ArduinoIoTCloudClass::push()
{
  requestUpdateForAllProperties(_property_container);
}

bool ArduinoIoTCloudClass::setTimestamp(String const & prop_name, unsigned long const timestamp)
{
  Property * p = getProperty(_property_container, prop_name);

  if (p == nullptr)
    return false;

  p->setTimestamp(timestamp);

  return true;
}

void ArduinoIoTCloudClass::addCallback(ArduinoIoTCloudEvent const event, OnCloudEventCallback callback)
{
  _cloud_event_callback[static_cast<size_t>(event)] = callback;
}

void ArduinoIoTCloudClass::addPropertyReal(Property& property, String name, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  addPropertyReal(property, name, -1, permission_type, seconds, fn, minDelta, synFn);
}

void ArduinoIoTCloudClass::addPropertyReal(Property& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
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
    addPropertyToContainer(_property_container, property, name, permission, tag).publishOnChange(minDelta, Property::DEFAULT_MIN_TIME_BETWEEN_UPDATES_MILLIS).onUpdate(fn).onSync(synFn);
  } else {
    addPropertyToContainer(_property_container, property, name, permission, tag).publishEvery(seconds).onUpdate(fn).onSync(synFn);
  }
}

Property& ArduinoIoTCloudClass::addPropertyReal(Property& property, String name, Permission const permission)
{
  return addPropertyToContainer(_property_container, property, name, permission);
}

Property& ArduinoIoTCloudClass::addPropertyReal(Property& property, String name, int tag, Permission const permission)
{
  return addPropertyToContainer(_property_container, property, name, permission, tag);
}

void ArduinoIoTCloudClass::addPropertyReal(bool& property, String name, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  addPropertyReal(property, name, -1, permission_type, seconds, fn, minDelta, synFn);
}

void ArduinoIoTCloudClass::addPropertyReal(bool& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  Property* p = new CloudWrapperBool(property);
  addPropertyReal(*p, name, tag, permission_type, seconds, fn, minDelta, synFn);
}

Property& ArduinoIoTCloudClass::addPropertyReal(bool& property, String name, Permission const permission)
{
  return addPropertyReal(property, name, -1, permission);
}

Property& ArduinoIoTCloudClass::addPropertyReal(bool& property, String name, int tag, Permission const permission)
{
  Property* p = new CloudWrapperBool(property);
  return addPropertyToContainer(_property_container, *p, name, permission, tag);
}

void ArduinoIoTCloudClass::addPropertyReal(float& property, String name, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  addPropertyReal(property, name, -1, permission_type, seconds, fn, minDelta, synFn);
}

void ArduinoIoTCloudClass::addPropertyReal(float& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  Property* p = new CloudWrapperFloat(property);
  addPropertyReal(*p, name, tag, permission_type, seconds, fn, minDelta, synFn);
}

Property& ArduinoIoTCloudClass::addPropertyReal(float& property, String name, Permission const permission)
{
  return addPropertyReal(property, name, -1, permission);
}

Property& ArduinoIoTCloudClass::addPropertyReal(float& property, String name, int tag, Permission const permission)
{
  Property* p = new CloudWrapperFloat(property);
  return addPropertyToContainer(_property_container, *p, name, permission, tag);
}

void ArduinoIoTCloudClass::addPropertyReal(int& property, String name, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  addPropertyReal(property, name, -1, permission_type, seconds, fn, minDelta, synFn);
}

void ArduinoIoTCloudClass::addPropertyReal(int& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  Property* p = new CloudWrapperInt(property);
  addPropertyReal(*p, name, tag, permission_type, seconds, fn, minDelta, synFn);
}

Property& ArduinoIoTCloudClass::addPropertyReal(int& property, String name, Permission const permission)
{
  return addPropertyReal(property, name, -1, permission);
}

Property& ArduinoIoTCloudClass::addPropertyReal(int& property, String name, int tag, Permission const permission)
{
  Property* p = new CloudWrapperInt(property);
  return addPropertyToContainer(_property_container, *p, name, permission, tag);
}

void ArduinoIoTCloudClass::addPropertyReal(unsigned int& property, String name, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  addPropertyReal(property, name, -1, permission_type, seconds, fn, minDelta, synFn);
}

void ArduinoIoTCloudClass::addPropertyReal(unsigned int& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  Property* p = new CloudWrapperUnsignedInt(property);
  addPropertyReal(*p, name, tag, permission_type, seconds, fn, minDelta, synFn);
}

Property& ArduinoIoTCloudClass::addPropertyReal(unsigned int& property, String name, Permission const permission)
{
  return addPropertyReal(property, name, -1, permission);
}

Property& ArduinoIoTCloudClass::addPropertyReal(unsigned int& property, String name, int tag, Permission const permission)
{
  Property* p = new CloudWrapperUnsignedInt(property);
  return addPropertyToContainer(_property_container, *p, name, permission, tag);
}

void ArduinoIoTCloudClass::addPropertyReal(String& property, String name, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  addPropertyReal(property, name, -1, permission_type, seconds, fn, minDelta, synFn);
}

void ArduinoIoTCloudClass::addPropertyReal(String& property, String name, int tag, permissionType permission_type, long seconds, void(*fn)(void), float minDelta, void(*synFn)(Property & property))
{
  Property* p = new CloudWrapperString(property);
  addPropertyReal(*p, name, tag, permission_type, seconds, fn, minDelta, synFn);
}

Property& ArduinoIoTCloudClass::addPropertyReal(String& property, String name, Permission const permission)
{
  return addPropertyReal(property, name, -1, permission);
}

Property& ArduinoIoTCloudClass::addPropertyReal(String& property, String name, int tag, Permission const permission)
{
  Property* p = new CloudWrapperString(property);
  return addPropertyToContainer(_property_container, *p, name, permission, tag);
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
