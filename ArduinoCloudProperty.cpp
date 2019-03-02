//
// This file is part of ArduinoCloudThing
//
// Copyright 2019 ARDUINO SA (http://www.arduino.cc/)
//
// This software is released under the GNU General Public License version 3,
// which covers the main part of ArduinoCloudThing.
// The terms of this license can be found at:
// https://www.gnu.org/licenses/gpl-3.0.en.html
//
// You can be released from the requirements of the above licenses by purchasing
// a commercial license. Buying such a license is mandatory if you want to modify or
// otherwise use the software for commercial activities involving the Arduino
// software without disclosing the source code of your own applications. To purchase
// a commercial license, send an email to license@arduino.cc.
//

#include "ArduinoCloudProperty.hpp"

#ifdef ARDUINO_ARCH_SAMD
  #include <RTCZero.h>
  extern RTCZero rtc;
#endif

static unsigned long getTimestamp() {
  #ifdef ARDUINO_ARCH_SAMD
    return rtc.getEpoch();
  #else
    #warning "No RTC available on this architecture - ArduinoIoTCloud will not keep track of local change timestamps ." 
    return 0;
  #endif
}

/******************************************************************************
 * CTOR/DTOR
 ******************************************************************************/
ArduinoCloudProperty::ArduinoCloudProperty()
: _name(""),
  _permission(Permission::Read),
  _update_callback_func(nullptr),
  _sync_callback_func(nullptr),
  _has_been_updated_once(false),
  _has_been_modified_in_callback(false),
  _min_delta_property(0.0f),
  _min_time_between_updates_millis(0),
  _last_updated_millis(0),
  _update_interval_millis(0),
  _last_local_change_timestamp(0),
  _last_cloud_change_timestamp(0)
{
}

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/
void ArduinoCloudProperty::init(String const name, Permission const permission) {
  _name = name;
  _permission = permission;
}

ArduinoCloudProperty & ArduinoCloudProperty::onUpdate(UpdateCallbackFunc func) {
  _update_callback_func = func;
  return (*this);
}

ArduinoCloudProperty & ArduinoCloudProperty::onSync(void (*func)(ArduinoCloudProperty & property)) {
  _sync_callback_func = func;
  return (*this);
}

ArduinoCloudProperty & ArduinoCloudProperty::publishOnChange(float const min_delta_property, unsigned long const min_time_between_updates_millis) {
  _update_policy = UpdatePolicy::OnChange;
  _min_delta_property = min_delta_property;
  _min_time_between_updates_millis = min_time_between_updates_millis;
  return (*this);
}

ArduinoCloudProperty & ArduinoCloudProperty::publishEvery(unsigned long const seconds) {
  _update_policy = UpdatePolicy::TimeInterval;
  _update_interval_millis = (seconds * 1000);
  return (*this);
}

bool ArduinoCloudProperty::shouldBeUpdated() {
  if(!_has_been_updated_once) return true;

  if(_has_been_modified_in_callback) {
    _has_been_modified_in_callback = false;
    return true;
  }

  if     (_update_policy == UpdatePolicy::OnChange) {
    return (isDifferentFromCloudShadow() && ((millis() - _last_updated_millis) >= (_min_time_between_updates_millis)));
  }
  else if(_update_policy == UpdatePolicy::TimeInterval) {
    return ((millis() - _last_updated_millis) >= _update_interval_millis);
  }
  else {
    return false;
  }
}

void ArduinoCloudProperty::execCallbackOnChange() {
  if(_update_callback_func != NULL) {
    _update_callback_func();
  }
  if(!isDifferentFromCloudShadow()) {
    _has_been_modified_in_callback = true;
  }
}

void ArduinoCloudProperty::execCallbackOnSync() {
  if(_sync_callback_func != NULL) {
    _sync_callback_func(*this);
  }
}

void ArduinoCloudProperty::append(CborEncoder * encoder) {
  if (isReadableByCloud()) {
    CborEncoder mapEncoder;

    cbor_encoder_create_map     (encoder, &mapEncoder, CborIndefiniteLength);
    cbor_encode_int             (&mapEncoder, static_cast<int>(CborIntegerMapKey::Name));    
    cbor_encode_text_stringz    (&mapEncoder, _name.c_str());
    appendValue                 (&mapEncoder);
    cbor_encoder_close_container(encoder, &mapEncoder);

    toShadow();
 
    _has_been_updated_once = true;
    _last_updated_millis = millis();
  }
}
void ArduinoCloudProperty::updateLocalTimestamp() {
  if (isReadableByCloud()) {
    _last_local_change_timestamp = getTimestamp();
//    ArduinoCloud.update();
  }
}

void ArduinoCloudProperty::setLastCloudChangeTimestamp(unsigned long cloudChangeEventTime) {
  _last_cloud_change_timestamp = cloudChangeEventTime;
}
void ArduinoCloudProperty::setLastLocalChangeTimestamp(unsigned long localChangeTime) {
  _last_local_change_timestamp = localChangeTime;
}

unsigned long ArduinoCloudProperty::getLastCloudChangeTimestamp() {
  return _last_cloud_change_timestamp;
}

unsigned long ArduinoCloudProperty::getLastLocalChangeTimestamp() {
  return _last_local_change_timestamp;
}


/******************************************************************************
 * PRIVATE MEMBER FUNCTIONS 
 ******************************************************************************/
