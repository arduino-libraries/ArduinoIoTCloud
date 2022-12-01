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

CborError Property::appendAttribute(bool value, String attributeName, CborEncoder *encoder) {
  return appendAttributeName(attributeName, [value](CborEncoder & mapEncoder)
  {
    CHECK_CBOR(cbor_encode_int(&mapEncoder, static_cast<int>(CborIntegerMapKey::BooleanValue)));
    CHECK_CBOR(cbor_encode_boolean(&mapEncoder, value));
    return CborNoError;
  }, encoder);
}

CborError Property::appendAttribute(int value, String attributeName, CborEncoder *encoder) {
  return appendAttributeName(attributeName, [value](CborEncoder & mapEncoder)
  {
    CHECK_CBOR(cbor_encode_int(&mapEncoder, static_cast<int>(CborIntegerMapKey::Value)));
    CHECK_CBOR(cbor_encode_int(&mapEncoder, value));
    return CborNoError;
  }, encoder);
}

CborError Property::appendAttribute(unsigned int value, String attributeName, CborEncoder *encoder) {
  return appendAttributeName(attributeName, [value](CborEncoder & mapEncoder)
  {
    CHECK_CBOR(cbor_encode_int(&mapEncoder, static_cast<int>(CborIntegerMapKey::Value)));
    CHECK_CBOR(cbor_encode_int(&mapEncoder, value));
    return CborNoError;
  }, encoder);
}

CborError Property::appendAttribute(float value, String attributeName, CborEncoder *encoder) {
  return appendAttributeName(attributeName, [value](CborEncoder & mapEncoder)
  {
    CHECK_CBOR(cbor_encode_int(&mapEncoder, static_cast<int>(CborIntegerMapKey::Value)));
    CHECK_CBOR(cbor_encode_float(&mapEncoder, value));
    return CborNoError;
  }, encoder);
}

CborError Property::appendAttribute(String value, String attributeName, CborEncoder *encoder) {
  return appendAttributeName(attributeName, [value](CborEncoder & mapEncoder)
  {
    CHECK_CBOR(cbor_encode_int(&mapEncoder, static_cast<int>(CborIntegerMapKey::StringValue)));
    CHECK_CBOR(cbor_encode_text_stringz(&mapEncoder, value.c_str()));
    return CborNoError;
  }, encoder);
}

#ifdef __AVR__
CborError Property::appendAttributeName(String attributeName, nonstd::function<CborError (CborEncoder& mapEncoder)>appendValue, CborEncoder *encoder)
#else
CborError Property::appendAttributeName(String attributeName, std::function<CborError (CborEncoder& mapEncoder)>appendValue, CborEncoder *encoder)
#endif
{
  if (attributeName != "") {
    // when the attribute name string is not empty, the attribute identifier is incremented in order to be encoded in the message if the _lightPayload flag is set
    _attributeIdentifier++;
  }
  CborEncoder mapEncoder;
  unsigned int num_map_properties = _encode_timestamp ? 3 : 2;
  CHECK_CBOR(cbor_encoder_create_map(encoder, &mapEncoder, num_map_properties));
  CHECK_CBOR(cbor_encode_int(&mapEncoder, static_cast<int>(CborIntegerMapKey::Name)));

  // if _lightPayload is true, the property and attribute identifiers will be encoded instead of the property name
  if (_lightPayload)
  {
    // the most significant byte of the identifier to be encoded represent the property identifier
    int completeIdentifier = _attributeIdentifier * 256;
    // the least significant byte of the identifier to be encoded represent the attribute identifier
    completeIdentifier += _identifier;
    CHECK_CBOR(cbor_encode_int(&mapEncoder, completeIdentifier));
  }
  else
  {
    String completeName = _name;
    if (attributeName != "") {
      completeName += ":" + attributeName;
    }
    CHECK_CBOR(cbor_encode_text_stringz(&mapEncoder, completeName.c_str()));
  }
  /* Encode the value */
  CHECK_CBOR(appendValue(mapEncoder));

  /* Encode the timestamp if that has been required. */
  if(_encode_timestamp)
  {
    CHECK_CBOR(cbor_encode_int (&mapEncoder, static_cast<int>(CborIntegerMapKey::Time)));
    CHECK_CBOR(cbor_encode_uint(&mapEncoder, _timestamp));
  }
  /* Close the container */
  CHECK_CBOR(cbor_encoder_close_container(encoder, &mapEncoder));
  return CborNoError;
}

void Property::setAttributesFromCloud(std::list<CborMapData> * map_data_list) {
  _map_data_list = map_data_list;
  _attributeIdentifier = 0;
  setAttributesFromCloud();
}

void Property::setAttribute(bool& value, String attributeName) {
  setAttribute(attributeName, [&value](CborMapData & md) {
    // Manage the case to have boolean values received as integers 0/1
    if (md.bool_val.isSet()) {
      value = md.bool_val.get();
    } else if (md.val.isSet()) {
      if (md.val.get() == 0) {
        value = false;
      } else if (md.val.get() == 1) {
        value = true;
      } else {
        /* This should not happen. Leave the previous value */
      }
    }
  });
}

void Property::setAttribute(int& value, String attributeName) {
  setAttribute(attributeName, [&value](CborMapData & md) {
    value = md.val.get();
  });
}

void Property::setAttribute(unsigned int& value, String attributeName) {
  setAttribute(attributeName, [&value](CborMapData & md) {
    value = md.val.get();
  });
}

void Property::setAttribute(float& value, String attributeName) {
  setAttribute(attributeName, [&value](CborMapData & md) {
    value = md.val.get();
  });
}

void Property::setAttribute(String& value, String attributeName) {
  setAttribute(attributeName, [&value](CborMapData & md) {
    value = md.str_val.get();
  });
}

#ifdef __AVR__
void Property::setAttribute(String attributeName, nonstd::function<void (CborMapData & md)>setValue)
#else
void Property::setAttribute(String attributeName, std::function<void (CborMapData & md)>setValue)
#endif
{
  if (attributeName != "") {
    _attributeIdentifier++;
  }

  std::for_each(_map_data_list->begin(),
                _map_data_list->end(),
                [this, attributeName, setValue](CborMapData & map)
                {
                  if (map.light_payload.isSet() && map.light_payload.get())
                  {
                    // if a light payload is detected, the attribute identifier is retrieved from the cbor map and the corresponding attribute is updated
                    int attid = map.attribute_identifier.get();
                    if (attid == _attributeIdentifier) {
                      setValue(map);
                      return;
                    }
                  }
                  else
                  {
                    // if a normal payload is detected, the name of the attribute to be updated is extracted directly from the cbor map
                    String an = map.attribute_name.get();
                    if (an == attributeName) {
                      setValue(map);
                      return;
                    }
                  }
                });
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
  property.fromCloudToLocal();
  property.execCallbackOnChange();
}

void onForceDeviceSync(Property & /* property */) {

}
