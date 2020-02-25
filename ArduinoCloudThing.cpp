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

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include <Arduino.h>

#include <ArduinoCloudThing.h>

/******************************************************************************
   DEBUG FUNCTIONS
 ******************************************************************************/

#if defined(DEBUG_MEMORY) && defined(ARDUINO_ARCH_SAMD)
extern "C" char *sbrk(int i);
void PrintFreeRam(void) {
  char stack_dummy = 0;
  //Serial.print("Free RAM: "); //Serial.println(&stack_dummy - sbrk(0));
}
#endif

#ifdef ARDUINO_ARCH_MRAA
  #define Serial DebugSerial
#endif

/******************************************************************************
   CTOR/DTOR
 ******************************************************************************/

ArduinoCloudThing::ArduinoCloudThing() :
  _get_time_func{nullptr},
  _numPrimitivesProperties(0),
  _numProperties(0),
  _isSyncMessage(false),
  _currentPropertyName(""),
  _currentPropertyBaseTime(0),
  _currentPropertyTime(0)
{}

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

void ArduinoCloudThing::begin() {
}

void ArduinoCloudThing::registerGetTimeCallbackFunc(GetTimeCallbackFunc func) {
  _get_time_func = func;
}

int ArduinoCloudThing::encode(uint8_t * data, size_t const size, bool lightPayload) {

  // check if backing storage and cloud has diverged
  // time interval may be elapsed or property may be changed
  CborEncoder encoder, arrayEncoder;

  cbor_encoder_init(&encoder, data, size, 0);

  if (cbor_encoder_create_array(&encoder, &arrayEncoder, CborIndefiniteLength) != CborNoError) {
    return -1;
  }

  if (appendChangedProperties(&arrayEncoder, lightPayload) < 1) {
    return -1;
  }

  if (cbor_encoder_close_container(&encoder, &arrayEncoder) != CborNoError) {
    return -1;
  }

  #if defined(DEBUG_MEMORY) && defined(ARDUINO_ARCH_SAMD)
  PrintFreeRam();
  #endif
  int const bytes_encoded = cbor_encoder_get_buffer_size(&encoder, data);
  return bytes_encoded;
}

ArduinoCloudProperty& ArduinoCloudThing::addPropertyReal(ArduinoCloudProperty & property, String const & name, Permission const permission, int propertyIdentifier) {
  property.init(name, permission, _get_time_func);
  if (isPropertyInContainer(name)) {
    return (*getProperty(name));
  } else {
    if (property.isPrimitive()) {
      _numPrimitivesProperties++;
    }
    _numProperties++;
    addProperty(&property, propertyIdentifier);
    return (property);
  }

}

void ArduinoCloudThing::decode(uint8_t const * const payload, size_t const length, bool isSyncMessage) {
  _isSyncMessage = isSyncMessage;

  CborParser parser;
  CborValue  array_iter,
             map_iter,
             value_iter;

  if (cbor_parser_init(payload, length, 0, &parser, &array_iter) != CborNoError) {
    return;
  }

  if (array_iter.type != CborArrayType) {
    return;
  }

  if (cbor_value_enter_container(&array_iter, &map_iter) != CborNoError) {
    return;
  }

  CborMapData    *map_data = nullptr;

  _map_data_list.clear();
  _currentPropertyName = "";

  MapParserState current_state = MapParserState::EnterMap,
                 next_state = MapParserState::Error;

  while (current_state != MapParserState::Complete) {

    switch (current_state) {
      case MapParserState::EnterMap     : next_state = handle_EnterMap(&map_iter, &value_iter, &map_data); break;
      case MapParserState::MapKey       : next_state = handle_MapKey(&value_iter); break;
      case MapParserState::UndefinedKey : next_state = handle_UndefinedKey(&value_iter); break;
      case MapParserState::BaseVersion  : next_state = handle_BaseVersion(&value_iter, map_data); break;
      case MapParserState::BaseName     : next_state = handle_BaseName(&value_iter, map_data); break;
      case MapParserState::BaseTime     : next_state = handle_BaseTime(&value_iter, map_data); break;
      case MapParserState::Time         : next_state = handle_Time(&value_iter, map_data); break;
      case MapParserState::Name         : next_state = handle_Name(&value_iter, map_data); break;
      case MapParserState::Value        : next_state = handle_Value(&value_iter, map_data); break;
      case MapParserState::StringValue  : next_state = handle_StringValue(&value_iter, map_data); break;
      case MapParserState::BooleanValue : next_state = handle_BooleanValue(&value_iter, map_data); break;
      case MapParserState::LeaveMap     : next_state = handle_LeaveMap(&map_iter, &value_iter, map_data); break;
      case MapParserState::Complete     : /* Nothing to do */                                                   break;
      case MapParserState::Error        : return;                                                               break;
    }

    current_state = next_state;
  }
}

bool ArduinoCloudThing::isPropertyInContainer(String const & name) {
  for (int i = 0; i < _property_list.size(); i++) {
    ArduinoCloudProperty * p = _property_list.get(i);
    if (p->name() == name) {
      return true;
    }
  }
  return false;
}

int ArduinoCloudThing::appendChangedProperties(CborEncoder * arrayEncoder, bool lightPayload) {
  int appendedProperties = 0;
  for (int i = 0; i < _property_list.size(); i++) {
    ArduinoCloudProperty * p = _property_list.get(i);
    if (p->shouldBeUpdated() && p->isReadableByCloud()) {
      p->append(arrayEncoder, lightPayload);
      appendedProperties++;
    }
  }
  return appendedProperties;
}

//retrieve property by name
ArduinoCloudProperty * ArduinoCloudThing::getProperty(String const & name) {
  for (int i = 0; i < _property_list.size(); i++) {
    ArduinoCloudProperty * p = _property_list.get(i);
    if (p->name() == name) {
      return p;
    }
  }
  return NULL;
}

//retrieve property by identifier
ArduinoCloudProperty * ArduinoCloudThing::getProperty(int const & pos) {
  for (int i = 0; i < _property_list.size(); i++) {
    ArduinoCloudProperty * p = _property_list.get(i);
    if (p->identifier() == pos) {
      return p;
    }
  }
  return NULL;
}

// this function updates the timestamps on the primitive properties that have been modified locally since last cloud synchronization
void ArduinoCloudThing::updateTimestampOnLocallyChangedProperties() {
  if (_numPrimitivesProperties == 0) {
    return;
  } else {
    for (int i = 0; i < _property_list.size(); i++) {
      CloudWrapperBase * p = (CloudWrapperBase *)_property_list.get(i);
      if (p->isPrimitive() && p->isChangedLocally() && p->isReadableByCloud()) {
        p->updateLocalTimestamp();
      }
    }
  }
}

/******************************************************************************
   PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

ArduinoCloudThing::MapParserState ArduinoCloudThing::handle_EnterMap(CborValue * map_iter, CborValue * value_iter, CborMapData **map_data) {
  MapParserState next_state = MapParserState::Error;

  if (cbor_value_get_type(map_iter) == CborMapType) {
    if (cbor_value_enter_container(map_iter, value_iter) == CborNoError) {
      *map_data = new CborMapData();
      next_state = MapParserState::MapKey;
    }
  }

  return next_state;
}

ArduinoCloudThing::MapParserState ArduinoCloudThing::handle_MapKey(CborValue * value_iter) {
  MapParserState next_state = MapParserState::Error;

  if (cbor_value_at_end(value_iter)) {
    next_state = MapParserState::LeaveMap;
  }
  /* The Map use the CBOR Label (protocol V2)
     Example [{0: "temperature", 2: 25}]
  */
  else if (cbor_value_is_integer(value_iter)) {
    int val = 0;
    if (cbor_value_get_int(value_iter, &val) == CborNoError) {
      if (cbor_value_advance(value_iter) == CborNoError) {
        if (val == static_cast<int>(CborIntegerMapKey::Name)) {
          next_state = MapParserState::Name;
        } else if (val == static_cast<int>(CborIntegerMapKey::BaseVersion)) {
          next_state = MapParserState::BaseVersion;
        } else if (val == static_cast<int>(CborIntegerMapKey::BaseName)) {
          next_state = MapParserState::BaseName;
        } else if (val == static_cast<int>(CborIntegerMapKey::BaseTime)) {
          next_state = MapParserState::BaseTime;
        } else if (val == static_cast<int>(CborIntegerMapKey::Value)) {
          next_state = MapParserState::Value;
        } else if (val == static_cast<int>(CborIntegerMapKey::StringValue)) {
          next_state = MapParserState::StringValue;
        } else if (val == static_cast<int>(CborIntegerMapKey::BooleanValue)) {
          next_state = MapParserState::BooleanValue;
        } else if (val == static_cast<int>(CborIntegerMapKey::Time)) {
          next_state = MapParserState::Time;
        } else {
          next_state = MapParserState::UndefinedKey;
        }
      }
    }
  }

  return next_state;
}

ArduinoCloudThing::MapParserState ArduinoCloudThing::handle_UndefinedKey(CborValue * value_iter) {
  MapParserState next_state = MapParserState::Error;

  if (cbor_value_advance(value_iter) == CborNoError) {
    next_state = MapParserState::MapKey;
  }

  return next_state;
}

ArduinoCloudThing::MapParserState ArduinoCloudThing::handle_BaseVersion(CborValue * value_iter, CborMapData * map_data) {
  MapParserState next_state = MapParserState::Error;

  if (cbor_value_is_integer(value_iter)) {
    int val = 0;
    if (cbor_value_get_int(value_iter, &val) == CborNoError) {
      map_data->base_version.set(val);

      if (cbor_value_advance(value_iter) == CborNoError) {
        next_state = MapParserState::MapKey;
      }
    }
  }

  return next_state;
}

ArduinoCloudThing::MapParserState ArduinoCloudThing::handle_BaseName(CborValue * value_iter, CborMapData * map_data) {
  MapParserState next_state = MapParserState::Error;

  if (cbor_value_is_text_string(value_iter)) {
    char * val      = 0;
    size_t val_size = 0;
    if (cbor_value_dup_text_string(value_iter, &val, &val_size, value_iter) == CborNoError) {
      map_data->base_name.set(String(val));
      free(val);
      next_state = MapParserState::MapKey;
    }
  }

  return next_state;
}

ArduinoCloudThing::MapParserState ArduinoCloudThing::handle_BaseTime(CborValue * value_iter, CborMapData * map_data) {
  MapParserState next_state = MapParserState::Error;

  double val = 0.0;
  if (ifNumericConvertToDouble(value_iter, &val)) {
    map_data->base_time.set(val);

    if (cbor_value_advance(value_iter) == CborNoError) {
      next_state = MapParserState::MapKey;
    }
  }

  return next_state;
}

ArduinoCloudThing::MapParserState ArduinoCloudThing::handle_Name(CborValue * value_iter, CborMapData * map_data) {
  MapParserState next_state = MapParserState::Error;

  if (cbor_value_is_text_string(value_iter)) {
    // if the value in the cbor message is a string, it corresponds to the name of the property to be updated (int the form [property_name]:[attribute_name])
    char * val      = nullptr;
    size_t val_size = 0;
    if (cbor_value_dup_text_string(value_iter, &val, &val_size, value_iter) == CborNoError) {
      String name = val;
      free(val);
      map_data->name.set(name);
      int colonPos = name.indexOf(":");
      String attribute_name = "";
      if (colonPos != -1) {
        attribute_name = name.substring(colonPos + 1);
      }
      map_data->attribute_name.set(attribute_name);
      next_state = MapParserState::MapKey;
    }
  } else if (cbor_value_is_integer(value_iter)) {
    // if the value in the cbor message is an integer, a light payload has been used and an integer identifier should be decode in order to retrieve the corresponding property and attribute name to be updated
    int val = 0;
    if (cbor_value_get_int(value_iter, &val) == CborNoError) {
      map_data->light_payload.set(true);
      map_data->name_identifier.set(val & 255);
      map_data->attribute_identifier.set(val >> 8);
      map_data->light_payload.set(true);
      String name = getPropertyNameByIdentifier(val);
      map_data->name.set(name);


      if (cbor_value_advance(value_iter) == CborNoError) {
        next_state = MapParserState::MapKey;
      }
    }
  }



  return next_state;
}

ArduinoCloudThing::MapParserState ArduinoCloudThing::handle_Value(CborValue * value_iter, CborMapData * map_data) {
  MapParserState next_state = MapParserState::Error;

  double val = 0.0;
  if (ifNumericConvertToDouble(value_iter, &val)) {
    map_data->val.set(val);

    if (cbor_value_advance(value_iter) == CborNoError) {
      next_state = MapParserState::MapKey;
    }
  }

  return next_state;
}

ArduinoCloudThing::MapParserState ArduinoCloudThing::handle_StringValue(CborValue * value_iter, CborMapData * map_data) {
  MapParserState next_state = MapParserState::Error;

  if (cbor_value_is_text_string(value_iter)) {
    char * val      = 0;
    size_t val_size = 0;
    if (cbor_value_dup_text_string(value_iter, &val, &val_size, value_iter) == CborNoError) {
      map_data->str_val.set(String(val));
      free(val);
      next_state = MapParserState::MapKey;
    }
  }

  return next_state;
}

ArduinoCloudThing::MapParserState ArduinoCloudThing::handle_BooleanValue(CborValue * value_iter, CborMapData * map_data) {
  MapParserState next_state = MapParserState::Error;

  bool val = false;
  if (cbor_value_get_boolean(value_iter, &val) == CborNoError) {
    map_data->bool_val.set(val);

    if (cbor_value_advance(value_iter) == CborNoError) {
      next_state = MapParserState::MapKey;
    }
  }

  return next_state;
}

ArduinoCloudThing::MapParserState ArduinoCloudThing::handle_Time(CborValue * value_iter, CborMapData * map_data) {
  MapParserState next_state = MapParserState::Error;

  double val = 0.0;
  if (ifNumericConvertToDouble(value_iter, &val)) {
    map_data->time.set(val);

    if (cbor_value_advance(value_iter) == CborNoError) {
      next_state = MapParserState::MapKey;
    }
  }

  return next_state;
}

ArduinoCloudThing::MapParserState ArduinoCloudThing::handle_LeaveMap(CborValue * map_iter, CborValue * value_iter, CborMapData * map_data) {
  MapParserState next_state = MapParserState::Error;
  if (map_data->name.isSet()) {
    String propertyName;
    int colonPos = map_data->name.get().indexOf(":");
    if (colonPos != -1) {
      propertyName = map_data->name.get().substring(0, colonPos);
    } else {
      propertyName = map_data->name.get();
    }

    if (_currentPropertyName != "" && propertyName != _currentPropertyName) {
      /* Update the property containers depending on the parsed data */
      updateProperty(_currentPropertyName, _currentPropertyBaseTime + _currentPropertyTime);
      /* Reset current property data */
      freeMapDataList(&_map_data_list);
      _map_data_list.clear();
      _currentPropertyBaseTime = 0;
      _currentPropertyTime = 0;
    }
    /* Compute the cloud change event baseTime and Time */
    if (map_data->base_time.isSet()) {
      _currentPropertyBaseTime = (unsigned long)(map_data->base_time.get());
    }
    if (map_data->time.isSet() && (map_data->time.get() > _currentPropertyTime)) {
      _currentPropertyTime = (unsigned long)map_data->time.get();
    }
    _map_data_list.add(map_data);
    _currentPropertyName = propertyName;
  }

  /* Transition into the next map if available, otherwise finish */
  if (cbor_value_leave_container(map_iter, value_iter) == CborNoError) {
    if (!cbor_value_at_end(map_iter)) {
      next_state = MapParserState::EnterMap;
    } else {
      /* Update the property containers depending on the parsed data */
      updateProperty(_currentPropertyName, _currentPropertyBaseTime + _currentPropertyTime);
      /* Reset last property data */
      freeMapDataList(&_map_data_list);
      _map_data_list.clear();
      next_state = MapParserState::Complete;
    }
  }

  return next_state;
}

void ArduinoCloudThing::freeMapDataList(LinkedList<CborMapData *> *map_data_list) {
  while (map_data_list->size() > 0) {
    CborMapData const * mapData = map_data_list->pop();
    delete mapData;
  }
}

void ArduinoCloudThing::updateProperty(String propertyName, unsigned long cloudChangeEventTime) {
  ArduinoCloudProperty* property = getProperty(propertyName);
  if (property && property->isWriteableByCloud()) {
    property->setLastCloudChangeTimestamp(cloudChangeEventTime);
    property->setAttributesFromCloud(&_map_data_list);
    if (_isSyncMessage) {
      property->execCallbackOnSync();
    } else {
      property->fromCloudToLocal();
      property->execCallbackOnChange();
    }
  }
}

// retrieve the property name by the identifier
String ArduinoCloudThing::getPropertyNameByIdentifier(int propertyIdentifier) {
  ArduinoCloudProperty* property;
  if (propertyIdentifier > 255) {
    property = getProperty(propertyIdentifier & 255);
  } else {
    property = getProperty(propertyIdentifier);
  }
  return property->name();
}

bool ArduinoCloudThing::ifNumericConvertToDouble(CborValue * value_iter, double * numeric_val) {

  if (cbor_value_is_integer(value_iter)) {
    int64_t val = 0;
    if (cbor_value_get_int64(value_iter, &val) == CborNoError) {
      *numeric_val = static_cast<double>(val);
      return true;
    }
  } else if (cbor_value_is_double(value_iter)) {
    double val = 0.0;
    if (cbor_value_get_double(value_iter, &val) == CborNoError) {
      *numeric_val = val;
      return true;
    }
  } else if (cbor_value_is_float(value_iter)) {
    float val = 0.0;
    if (cbor_value_get_float(value_iter, &val) == CborNoError) {
      *numeric_val = static_cast<double>(val);
      return true;
    }
  } else if (cbor_value_is_half_float(value_iter)) {
    uint16_t val = 0;
    if (cbor_value_get_half_float(value_iter, &val) == CborNoError) {
      *numeric_val = static_cast<double>(convertCborHalfFloatToDouble(val));
      return true;
    }
  }

  return false;
}

/* Source Idea from https://tools.ietf.org/html/rfc7049 : Page: 50 */
double ArduinoCloudThing::convertCborHalfFloatToDouble(uint16_t const half_val) {
  int exp = (half_val >> 10) & 0x1f;
  int mant = half_val & 0x3ff;
  double val;
  if (exp == 0) {
    val = ldexp(mant, -24);
  } else if (exp != 31) {
    val = ldexp(mant + 1024, exp - 25);
  } else {
    val = mant == 0 ? INFINITY : NAN;
  }
  return half_val & 0x8000 ? -val : val;
}

void onAutoSync(ArduinoCloudProperty & property) {
  if (property.getLastCloudChangeTimestamp() > property.getLastLocalChangeTimestamp()) {
    property.fromCloudToLocal();
    property.execCallbackOnChange();
  }
}

void onForceCloudSync(ArduinoCloudProperty & property) {
  property.fromCloudToLocal();
  property.execCallbackOnChange();
}

void onForceDeviceSync(ArduinoCloudProperty & /* property */) {
}
