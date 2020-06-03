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

#include "ArduinoCloudProperty.h"

#ifndef ARDUINO_ARCH_SAMD
  #pragma message "No RTC available on this architecture - ArduinoIoTCloud will not keep track of local change timestamps ."
#endif

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/
ArduinoCloudProperty::ArduinoCloudProperty()
  :   _name(""),
      _min_delta_property(0.0f),
      _min_time_between_updates_millis(0),
      _permission(Permission::Read),
      _get_time_func{nullptr},
      _update_callback_func(nullptr),
      _sync_callback_func(nullptr),
      _has_been_updated_once(false),
      _has_been_modified_in_callback(false),
      _last_updated_millis(0),
      _update_interval_millis(0),
      _last_local_change_timestamp(0),
      _last_cloud_change_timestamp(0),
      _map_data_list(nullptr),
      _identifier(0),
      _attributeIdentifier(0),
      _lightPayload(false) {
}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/
void ArduinoCloudProperty::init(String const name, Permission const permission, GetTimeCallbackFunc func) {
  _name = name;
  _permission = permission;
  _get_time_func = func;
}

ArduinoCloudProperty & ArduinoCloudProperty::onUpdate(UpdateCallbackFunc func) {
  _update_callback_func = func;
  return (*this);
}

ArduinoCloudProperty & ArduinoCloudProperty::onSync(SyncCallbackFunc func) {
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
  if (!_has_been_updated_once) {
    return true;
  }

  if (_has_been_modified_in_callback) {
    _has_been_modified_in_callback = false;
    return true;
  }

  if (_update_policy == UpdatePolicy::OnChange) {
    return (isDifferentFromCloud() && ((millis() - _last_updated_millis) >= (_min_time_between_updates_millis)));
  } else if (_update_policy == UpdatePolicy::TimeInterval) {
    return ((millis() - _last_updated_millis) >= _update_interval_millis);
  } else {
    return false;
  }
}

void ArduinoCloudProperty::execCallbackOnChange() {
  if (_update_callback_func != NULL) {
    _update_callback_func();
  }
  if (!isDifferentFromCloud()) {
    _has_been_modified_in_callback = true;
  }
}

void ArduinoCloudProperty::execCallbackOnSync() {
  if (_sync_callback_func != NULL) {
    _sync_callback_func(*this);
  }
}

void ArduinoCloudProperty::append(CborEncoder *encoder, bool lightPayload) {
  _lightPayload = lightPayload;
  _attributeIdentifier = 0;
  appendAttributesToCloudReal(encoder);
  fromLocalToCloud();
  _has_been_updated_once = true;
  _last_updated_millis = millis();
}

void ArduinoCloudProperty::appendAttributeReal(bool value, String attributeName, CborEncoder *encoder) {
  appendAttributeName(attributeName, [value](CborEncoder & mapEncoder) {
    cbor_encode_int(&mapEncoder, static_cast<int>(CborIntegerMapKey::BooleanValue));
    cbor_encode_boolean(&mapEncoder, value);
  }, encoder);
}

void ArduinoCloudProperty::appendAttributeReal(int value, String attributeName, CborEncoder *encoder) {
  appendAttributeName(attributeName, [value](CborEncoder & mapEncoder) {
    cbor_encode_int(&mapEncoder, static_cast<int>(CborIntegerMapKey::Value));
    cbor_encode_int(&mapEncoder, value);
  }, encoder);
}

void ArduinoCloudProperty::appendAttributeReal(float value, String attributeName, CborEncoder *encoder) {
  appendAttributeName(attributeName, [value](CborEncoder & mapEncoder) {
    cbor_encode_int(&mapEncoder, static_cast<int>(CborIntegerMapKey::Value));
    cbor_encode_float(&mapEncoder, value);
  }, encoder);
}

void ArduinoCloudProperty::appendAttributeReal(String value, String attributeName, CborEncoder *encoder) {
  appendAttributeName(attributeName, [value](CborEncoder & mapEncoder) {
    cbor_encode_int(&mapEncoder, static_cast<int>(CborIntegerMapKey::StringValue));
    cbor_encode_text_stringz(&mapEncoder, value.c_str());
  }, encoder);
}

void ArduinoCloudProperty::appendAttributeName(String attributeName, std::function<void (CborEncoder& mapEncoder)>appendValue, CborEncoder *encoder) {
  if (attributeName != "") {
    // when the attribute name string is not empty, the attribute identifier is incremented in order to be encoded in the message if the _lightPayload flag is set
    _attributeIdentifier++;
  }
  CborEncoder mapEncoder;
  cbor_encoder_create_map(encoder, &mapEncoder, 2);
  cbor_encode_int(&mapEncoder, static_cast<int>(CborIntegerMapKey::Name));

  // if _lightPayload is true, the property and attribute identifiers will be encoded instead of the property name
  if (_lightPayload) {
    // the most significant byte of the identifier to be encoded represent the property identifier
    int completeIdentifier = _attributeIdentifier * 256;
    // the least significant byte of the identifier to be encoded represent the attribute identifier
    completeIdentifier += _identifier;
    cbor_encode_int(&mapEncoder, completeIdentifier);
  } else {
    String completeName = _name;
    if (attributeName != "") {
      completeName += ":" + attributeName;
    }
    cbor_encode_text_stringz(&mapEncoder, completeName.c_str());
  }
  appendValue(mapEncoder);
  cbor_encoder_close_container(encoder, &mapEncoder);
}

void ArduinoCloudProperty::setAttributesFromCloud(LinkedList<CborMapData *> *map_data_list) {
  _map_data_list = map_data_list;
  _attributeIdentifier = 0;
  setAttributesFromCloud();
}

void ArduinoCloudProperty::setAttributeReal(bool& value, String attributeName) {
  setAttributeReal(attributeName, [&value](CborMapData * md) {
    // Manage the case to have boolean values received as integers 0/1
    if (md->bool_val.isSet()) {
      value = md->bool_val.get();
    } else if (md->val.isSet()) {
      if (md->val.get() == 0) {
        value = false;
      } else if (md->val.get() == 1) {
        value = true;
      } else {
        /* This should not happen. Leave the previous value */
      }
    }
  });
}

void ArduinoCloudProperty::setAttributeReal(int& value, String attributeName) {
  setAttributeReal(attributeName, [&value](CborMapData * md) {
    value = md->val.get();
  });
}

void ArduinoCloudProperty::setAttributeReal(float& value, String attributeName) {
  setAttributeReal(attributeName, [&value](CborMapData * md) {
    value = md->val.get();
  });
}

void ArduinoCloudProperty::setAttributeReal(String& value, String attributeName) {
  setAttributeReal(attributeName, [&value](CborMapData * md) {
    value = md->str_val.get();
  });
}

void ArduinoCloudProperty::setAttributeReal(String attributeName, std::function<void (CborMapData *md)>setValue) {
  if (attributeName != "") {
    _attributeIdentifier++;
  }
  for (int i = 0; i < _map_data_list->size(); i++) {
    CborMapData *map = _map_data_list->get(i);
    if (map != nullptr) {
      if (map->light_payload.isSet() && map->light_payload.get()) {
        // if a light payload is detected, the attribute identifier is retrieved from the cbor map and the corresponding attribute is updated
        int attid = map->attribute_identifier.get();
        if (attid == _attributeIdentifier) {
          setValue(map);
          break;
        }
      } else {
        // if a normal payload is detected, the name of the attribute to be updated is extracted directly from the cbor map
        String an = map->attribute_name.get();
        if (an == attributeName) {
          setValue(map);
          break;
        }
      }
    }
  }

}

String ArduinoCloudProperty::getAttributeName(String propertyName, char separator) {
  int colonPos;
  String attributeName = "";
  (colonPos = propertyName.indexOf(separator)) != -1 ? attributeName = propertyName.substring(colonPos + 1) : "";
  return attributeName;
}

void ArduinoCloudProperty::updateLocalTimestamp() {
  if (isReadableByCloud()) {
    if (_get_time_func) {
      _last_local_change_timestamp = _get_time_func();
    }
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

void ArduinoCloudProperty::setIdentifier(int identifier) {
  _identifier = identifier;
}
