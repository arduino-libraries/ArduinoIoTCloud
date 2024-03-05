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

#include "Property.h"

#undef max
#undef min
#include <algorithm>

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/
Property::Property()
: _name{""}
, _min_delta_property{0.0f}
, _min_time_between_updates_millis{DEFAULT_MIN_TIME_BETWEEN_UPDATES_MILLIS}
, _permission{Permission::Read}
, _write_policy{WritePolicy::Auto}
, _get_time_func{nullptr}
, _update_callback_func{nullptr}
, _on_sync_callback_func{nullptr}
, _update_policy{UpdatePolicy::OnChange}
, _has_been_updated_once{false}
, _has_been_modified_in_callback{false}
, _has_been_appended_but_not_sended{false}
, _last_updated_millis{0}
, _update_interval_millis{0}
, _last_local_change_timestamp{0}
, _last_cloud_change_timestamp{0}
, _identifier{0}
, _attributeIdentifier{0}
, _lightPayload{false}
, _update_requested{false}
, _encode_timestamp{false}
, _echo_requested{false}
, _timestamp{0}
{

}

/******************************************************************************
   CONST
 ******************************************************************************/
namespace PropertyActions
{
  const String CLEAR = "\x1b";
}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/
void Property::init(String const name, Permission const permission, GetTimeCallbackFunc func) {
  _name = name;
  _permission = permission;
  _get_time_func = func;
}

Property & Property::onUpdate(UpdateCallbackFunc func) {
  _update_callback_func = func;
  return (*this);
}

Property & Property::onSync(OnSyncCallbackFunc func) {
  _on_sync_callback_func = func;
  return (*this);
}

Property & Property::publishOnChange(float const min_delta_property, unsigned long const min_time_between_updates_millis) {
  _update_policy = UpdatePolicy::OnChange;
  _min_delta_property = min_delta_property;
  _min_time_between_updates_millis = min_time_between_updates_millis;
  return (*this);
}

Property & Property::publishEvery(unsigned long const seconds) {
  _update_policy = UpdatePolicy::TimeInterval;
  _update_interval_millis = (seconds * 1000);
  return (*this);
}

Property & Property::publishOnDemand() {
  _update_policy = UpdatePolicy::OnDemand;
  return (*this);
}

Property & Property::encodeTimestamp()
{
  _encode_timestamp = true;
  return (*this);
}

Property & Property::writeOnChange()
{
  _write_policy = WritePolicy::Auto;
  return (*this);
}

Property & Property::writeOnDemand()
{
  _write_policy = WritePolicy::Manual;
  return (*this);
}

void Property::setTimestamp(unsigned long const timestamp)
{
  _timestamp = timestamp;
}

bool Property::shouldBeUpdated() {
  if (!_has_been_updated_once) {
    return true;
  }

  if (_has_been_appended_but_not_sended) {
    return true;
  }

  if (_has_been_modified_in_callback) {
    return true;
  }

  if (_echo_requested) {
    return true;
  }

  if (_update_policy == UpdatePolicy::OnChange) {
    return (isDifferentFromCloud() && ((millis() - _last_updated_millis) >= (_min_time_between_updates_millis)));
  } else if (_update_policy == UpdatePolicy::TimeInterval) {
    return ((millis() - _last_updated_millis) >= _update_interval_millis);
  } else if (_update_policy == UpdatePolicy::OnDemand) {
    return _update_requested;
  } else {
    return false;
  }
}

void Property::requestUpdate()
{
  _update_requested = true;
}

void Property::provideEcho()
{
  _echo_requested = true;
}

void Property::appendCompleted()
{
  if (_has_been_appended_but_not_sended) {
    _has_been_appended_but_not_sended = false;
  }
}

void Property::execCallbackOnChange() {
  if (_update_callback_func != nullptr) {
    _update_callback_func();
  }
  if (isDifferentFromCloud()) {
    _has_been_modified_in_callback = true;
  }
}

void Property::execCallbackOnSync() {
  if (_on_sync_callback_func != nullptr) {
    _on_sync_callback_func(*this);
  }
}

CborError Property::append(CborEncoder *encoder, bool lightPayload) {
  _lightPayload = lightPayload;
  _attributeIdentifier = 0;
  CHECK_CBOR(appendAttributesToCloud(encoder));
  fromLocalToCloud();
  _has_been_updated_once = true;
  _has_been_modified_in_callback = false;
  _update_requested = false;
  _echo_requested = false;
  _has_been_appended_but_not_sended = true;
  _last_updated_millis = millis();
  return CborNoError;
}

void Property::setAttributesFromCloud(std::list<CborMapData> * map_data_list) {
  _map_data_list = map_data_list;
  _attributeIdentifier = 0;
  setAttributesFromCloud();
}

void Property::updateLocalTimestamp() {
  if (isReadableByCloud()) {
    if (_get_time_func) {
      _last_local_change_timestamp = _get_time_func();
    }
  }
}

void Property::setLastCloudChangeTimestamp(unsigned long cloudChangeEventTime) {
  _last_cloud_change_timestamp = cloudChangeEventTime;
}

void Property::setLastLocalChangeTimestamp(unsigned long localChangeTime) {
  _last_local_change_timestamp = localChangeTime;
}

unsigned long Property::getLastCloudChangeTimestamp() {
  return _last_cloud_change_timestamp;
}

unsigned long Property::getLastLocalChangeTimestamp() {
  return _last_local_change_timestamp;
}

void Property::setIdentifier(int identifier) {
  _identifier = identifier;
}

/******************************************************************************
   SYNCHRONIZATION CALLBACKS
 ******************************************************************************/

void onAutoSync(Property & property) {
  if (property.getLastCloudChangeTimestamp() > property.getLastLocalChangeTimestamp()) {
    property.fromCloudToLocal();
    property.execCallbackOnChange();
  }
}

void onForceCloudSync(Property & property) {
  if (property.isDifferentFromCloud()) {
    property.fromCloudToLocal();
    property.execCallbackOnChange();
  }
}

void onForceDeviceSync(Property & /* property */) {

}
