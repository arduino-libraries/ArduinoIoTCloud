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
 * INCLUDE
 ******************************************************************************/

#include <Arduino.h>

#include <ArduinoCloudThing.h>

#include <math.h>

/******************************************************************************
 * DEBUG FUNCTIONS
 ******************************************************************************/

#if defined(DEBUG_MEMORY) && defined(ARDUINO_ARCH_SAMD)
extern "C" char *sbrk(int i);
void PrintFreeRam (void)
{
    char stack_dummy = 0;
    Serial.print("Free RAM: "); Serial.println(&stack_dummy - sbrk(0));
}
#endif

#ifdef ARDUINO_ARCH_SAMD
static void utox8(uint32_t val, char* s) {
    for (int i = 0; i < 8; i++) {
        int d = val & 0XF;
        val = (val >> 4);
        s[7 - i] = d > 9 ? 'A' + d - 10 : '0' + d;
    }
}
#endif

#ifdef ARDUINO_ARCH_MRAA
#define Serial DebugSerial
#endif

/******************************************************************************
 * CTOR/DTOR
 ******************************************************************************/

ArduinoCloudThing::ArduinoCloudThing(CloudProtocol const cloud_protocol)
: _cloud_protocol(cloud_protocol) {
#ifdef ARDUINO_ARCH_SAMD
    #define SERIAL_NUMBER_WORD_0    *(volatile uint32_t*)(0x0080A00C)
    #define SERIAL_NUMBER_WORD_1    *(volatile uint32_t*)(0x0080A040)
    #define SERIAL_NUMBER_WORD_2    *(volatile uint32_t*)(0x0080A044)
    #define SERIAL_NUMBER_WORD_3    *(volatile uint32_t*)(0x0080A048)

    utox8(SERIAL_NUMBER_WORD_0, &_uuid[0]);
    utox8(SERIAL_NUMBER_WORD_1, &_uuid[8]);
    utox8(SERIAL_NUMBER_WORD_2, &_uuid[16]);
    utox8(SERIAL_NUMBER_WORD_3, &_uuid[24]);
    _uuid[32] = '\0';
#endif
}

/******************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

void ArduinoCloudThing::begin() {
  _status = ON;
  addPropertyReal(_status, "status", Permission::Read);
}


int ArduinoCloudThing::encode(uint8_t * data, size_t const size) {

  // check if backing storage and cloud has diverged
  // time interval may be elapsed or property may be changed
  int const num_changed_properties = _property_cont.getNumOfChangedProperties();

  if (num_changed_properties > 0) {
      CborError err;
      CborEncoder encoder, arrayEncoder;

      cbor_encoder_init(&encoder, data, size, 0);
      // create a cbor array containing the property that should be updated.
      err = cbor_encoder_create_array(&encoder, &arrayEncoder, num_changed_properties);
      if (err) {
          //Serial.println(cbor_error_string(err));
          return -1;
      }

      _property_cont.appendChangedProperties(&arrayEncoder, _cloud_protocol);

      err = cbor_encoder_close_container(&encoder, &arrayEncoder);

      // return the number of byte of the CBOR encoded array
      return cbor_encoder_get_buffer_size(&encoder, data);
  }

#if defined(DEBUG_MEMORY) && defined(ARDUINO_ARCH_SAMD)
  PrintFreeRam();
#endif
  // If nothing has to be sent, return diff, that is 0 in this case
  return num_changed_properties;
}

ArduinoCloudProperty<bool> & ArduinoCloudThing::addPropertyReal(bool & property, String const & name, Permission const permission) {
  if(_property_cont.isPropertyInContainer(Type::Bool, name)) {
    return (*_property_cont.getPropertyBool(name));
  }
  else {
    ArduinoCloudProperty<bool> *property_opj = new ArduinoCloudProperty<bool>(property, name, permission);
    _property_cont.addProperty(property_opj);
    return (*property_opj);
  }
}

ArduinoCloudProperty<int> & ArduinoCloudThing::addPropertyReal(int & property, String const & name, Permission const permission) {
  if(_property_cont.isPropertyInContainer(Type::Int, name)) {
    return (*_property_cont.getPropertyInt(name));
  }
  else {
    ArduinoCloudProperty<int> * property_opj = new ArduinoCloudProperty<int>(property, name, permission);
    _property_cont.addProperty(property_opj);
    return (*property_opj);
  }
}

ArduinoCloudProperty<float> & ArduinoCloudThing::addPropertyReal(float & property, String const & name, Permission const permission) {
  if(_property_cont.isPropertyInContainer(Type::Float, name)) {
    return (*_property_cont.getPropertyFloat(name));
  }
  else {
    ArduinoCloudProperty<float> * property_opj = new ArduinoCloudProperty<float>(property, name, permission);
    _property_cont.addProperty(property_opj);
    return (*property_opj);
  }
}

ArduinoCloudProperty<String> & ArduinoCloudThing::addPropertyReal(String & property, String const & name, Permission const permission) {
  if(_property_cont.isPropertyInContainer(Type::String, name)) {
    return (*_property_cont.getPropertyString(name));
  }
  else {
    ArduinoCloudProperty<String> * property_opj = new ArduinoCloudProperty<String>(property, name, permission);
    _property_cont.addProperty(property_opj);
    return (*property_opj);
  }
}

void ArduinoCloudThing::decode(uint8_t const * const payload, size_t const length) {

  CborParser parser;
  CborValue  array_iter,
             map_iter,
             value_iter;

  if(cbor_parser_init(payload, length, 0, &parser, &array_iter) != CborNoError)
    return;

  if(array_iter.type != CborArrayType)
    return;

  if(cbor_value_enter_container(&array_iter, &map_iter) != CborNoError)
    return;

  MapData        map_data;
  MapParserState current_state = MapParserState::EnterMap,
                 next_state;

  while(current_state != MapParserState::Complete) {

    switch(current_state) {
    case MapParserState::EnterMap     : next_state = handle_EnterMap     (&map_iter, &value_iter, &map_data); break;
    case MapParserState::MapKey       : next_state = handle_MapKey       (&value_iter                      ); break;
    case MapParserState::UndefinedKey : next_state = handle_UndefinedKey (&value_iter                      ); break;
    case MapParserState::BaseVersion  : next_state = handle_BaseVersion  (&value_iter, &map_data           ); break;
    case MapParserState::BaseName     : next_state = handle_BaseName     (&value_iter, &map_data           ); break;
    case MapParserState::BaseTime     : next_state = handle_BaseTime     (&value_iter, &map_data           ); break;
    case MapParserState::Time         : next_state = handle_Time         (&value_iter, &map_data           ); break;
    case MapParserState::Name         : next_state = handle_Name         (&value_iter, &map_data           ); break;
    case MapParserState::Value        : next_state = handle_Value        (&value_iter, &map_data           ); break;
    case MapParserState::StringValue  : next_state = handle_StringValue  (&value_iter, &map_data           ); break;
    case MapParserState::BooleanValue : next_state = handle_BooleanValue (&value_iter, &map_data           ); break;
    case MapParserState::LeaveMap     : next_state = handle_LeaveMap     (&map_iter, &value_iter, &map_data); break;
    case MapParserState::Complete     : /* Nothing to do */                                                   break;
    case MapParserState::Error        : return;                                                               break;
    }

    current_state = next_state;
  }
}

/******************************************************************************
 * PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

ArduinoCloudThing::MapParserState ArduinoCloudThing::handle_EnterMap(CborValue * map_iter, CborValue * value_iter, MapData * map_data) {
  MapParserState next_state = MapParserState::Error;

  if(cbor_value_get_type(map_iter) == CborMapType) {
    if(cbor_value_enter_container(map_iter, value_iter) == CborNoError) {
      resetMapData(map_data);
      next_state = MapParserState::MapKey;
    }
  }

  return next_state;
}

ArduinoCloudThing::MapParserState ArduinoCloudThing::handle_MapKey(CborValue * value_iter) {
  MapParserState next_state = MapParserState::Error;

  if(cbor_value_at_end(value_iter)) {
    next_state = MapParserState::LeaveMap;
  }
  else {
    if(_cloud_protocol == CloudProtocol::V1) {
      if(cbor_value_is_text_string(value_iter)) {
        char * val      = 0;
        size_t val_size = 0;
        if(cbor_value_dup_text_string(value_iter, &val, &val_size, value_iter) == CborNoError) {
          if     (strcmp(val, "n"   ) == 0) next_state = MapParserState::Name;
          else if(strcmp(val, "bver") == 0) next_state = MapParserState::BaseVersion;
          else if(strcmp(val, "bn"  ) == 0) next_state = MapParserState::BaseName;
          else if(strcmp(val, "bt"  ) == 0) next_state = MapParserState::BaseTime;
          else if(strcmp(val, "v"   ) == 0) next_state = MapParserState::Value;
          else if(strcmp(val, "vs"  ) == 0) next_state = MapParserState::StringValue;
          else if(strcmp(val, "vb"  ) == 0) next_state = MapParserState::BooleanValue;
          else if(strcmp(val, "t"   ) == 0) next_state = MapParserState::Time;
          else                              next_state = MapParserState::UndefinedKey;
          free(val);
        }
      }
    }

    if(_cloud_protocol == CloudProtocol::V2) {
      if(cbor_value_is_integer(value_iter)) {
        int val = 0;
        if(cbor_value_get_int(value_iter, &val) == CborNoError) {
          if(cbor_value_advance(value_iter) == CborNoError) {
            if     (val == static_cast<int>(CborIntegerMapKey::Name        )) next_state = MapParserState::Name;
            else if(val == static_cast<int>(CborIntegerMapKey::BaseVersion )) next_state = MapParserState::BaseVersion;
            else if(val == static_cast<int>(CborIntegerMapKey::BaseName    )) next_state = MapParserState::BaseName;
            else if(val == static_cast<int>(CborIntegerMapKey::BaseTime    )) next_state = MapParserState::BaseTime;
            else if(val == static_cast<int>(CborIntegerMapKey::Value       )) next_state = MapParserState::Value;
            else if(val == static_cast<int>(CborIntegerMapKey::StringValue )) next_state = MapParserState::StringValue;
            else if(val == static_cast<int>(CborIntegerMapKey::BooleanValue)) next_state = MapParserState::BooleanValue;
            else if(val == static_cast<int>(CborIntegerMapKey::Time        )) next_state = MapParserState::Time;
            else                                                              next_state = MapParserState::UndefinedKey;
          }
        }
      }
    }
  }

  return next_state;
}

ArduinoCloudThing::MapParserState ArduinoCloudThing::handle_UndefinedKey(CborValue * value_iter) {
  MapParserState next_state = MapParserState::Error;

  if(cbor_value_advance(value_iter) == CborNoError) {
    next_state = MapParserState::MapKey;
  }

  return next_state;
}

ArduinoCloudThing::MapParserState ArduinoCloudThing::handle_BaseVersion(CborValue * value_iter, MapData * map_data) {
  MapParserState next_state = MapParserState::Error;

  if(cbor_value_is_integer(value_iter)) {
    int val = 0;
    if(cbor_value_get_int(value_iter, &val) == CborNoError) {
      map_data->base_version.set(val);
      if(cbor_value_advance(value_iter) == CborNoError) {
        next_state = MapParserState::MapKey;
      }
    }
  }

  return next_state;
}

ArduinoCloudThing::MapParserState ArduinoCloudThing::handle_BaseName(CborValue * value_iter, MapData * map_data) {
  MapParserState next_state = MapParserState::Error;

  if(cbor_value_is_text_string(value_iter)) {
    char * val      = 0;
    size_t val_size = 0;
    if(cbor_value_dup_text_string(value_iter, &val, &val_size, value_iter) == CborNoError) {
      map_data->base_name.set(String(val));
      free(val);
      next_state = MapParserState::MapKey;
    }
  }

  return next_state;
}

ArduinoCloudThing::MapParserState ArduinoCloudThing::handle_BaseTime(CborValue * value_iter, MapData * map_data) {
  MapParserState next_state = MapParserState::Error;

  if(cbor_value_is_integer(value_iter)) {
    int val = 0;
    if(cbor_value_get_int(value_iter, &val) == CborNoError) {
      map_data->base_time.set(static_cast<double>(val));
    }
  }

  if(cbor_value_is_double(value_iter)) {
    double val = 0.0;
    if(cbor_value_get_double(value_iter, &val) == CborNoError) {
      map_data->base_time.set(val);
    }
  }

  if(cbor_value_is_float(value_iter)) {
    float val = 0.0;
    if(cbor_value_get_float(value_iter, &val) == CborNoError) {
      map_data->base_time.set(static_cast<double>(val));
    }
  }

  if(cbor_value_is_half_float(value_iter)) {
    uint16_t val = 0;
    if(cbor_value_get_half_float(value_iter, &val) == CborNoError) {
      map_data->base_time.set(static_cast<double>(convertCborHalfFloatToDouble(val)));
    }
  }

  if(cbor_value_advance(value_iter) == CborNoError) {
    next_state = MapParserState::MapKey;
  }

  return next_state;
}

ArduinoCloudThing::MapParserState ArduinoCloudThing::handle_Name(CborValue * value_iter, MapData * map_data) {
  MapParserState next_state = MapParserState::Error;

  if(cbor_value_is_text_string(value_iter)) {
    char * val      = 0;
    size_t val_size = 0;
    if(cbor_value_dup_text_string(value_iter, &val, &val_size, value_iter) == CborNoError) {
      map_data->name.set(val);
      free(val);
      next_state = MapParserState::MapKey;
    }
  }

  return next_state;
}

ArduinoCloudThing::MapParserState ArduinoCloudThing::handle_Value(CborValue * value_iter, MapData * map_data) {
  MapParserState next_state = MapParserState::Error;

  if(value_iter->type == CborIntegerType) {
    int val = 0;
    if(cbor_value_get_int(value_iter, &val) == CborNoError) {
      map_data->val.set(static_cast<float>(val));
    }
  }
  else if(value_iter->type == CborDoubleType) {
    double val = 0.0;
    if(cbor_value_get_double(value_iter, &val) == CborNoError) {
      map_data->val.set(static_cast<float>(val));
    }
  }
  else if(value_iter->type == CborFloatType) {
    float val = 0.0f;
    if(cbor_value_get_float(value_iter, &val) == CborNoError) {
      map_data->val.set(val);
    }
  }
  else if(value_iter->type == CborHalfFloatType) {
    uint16_t val = 0;
    if(cbor_value_get_half_float(value_iter, &val) == CborNoError) {
      map_data->val.set(static_cast<float>(convertCborHalfFloatToDouble(val)));
    }
  }

  if(cbor_value_advance(value_iter) == CborNoError) {
    next_state = MapParserState::MapKey;
  }

  return next_state;
}

ArduinoCloudThing::MapParserState ArduinoCloudThing::handle_StringValue(CborValue * value_iter, MapData * map_data) {
  MapParserState next_state = MapParserState::Error;

  if(cbor_value_is_text_string(value_iter)) {
    char * val      = 0;
    size_t val_size = 0;
    if(cbor_value_dup_text_string(value_iter, &val, &val_size, value_iter) == CborNoError) {
      map_data->str_val.set(String(val));
      free(val);
      next_state = MapParserState::MapKey;
    }
  }

  return next_state;
}

ArduinoCloudThing::MapParserState ArduinoCloudThing::handle_BooleanValue(CborValue * value_iter, MapData * map_data) {
  MapParserState next_state = MapParserState::Error;

  bool val = false;
  if(cbor_value_get_boolean(value_iter, &val) == CborNoError) {
    map_data->bool_val.set(val);
    if(cbor_value_advance(value_iter) == CborNoError) {
      next_state = MapParserState::MapKey;
    }
  }

  return next_state;
}

ArduinoCloudThing::MapParserState ArduinoCloudThing::handle_Time(CborValue * value_iter, MapData * map_data) {
  MapParserState next_state = MapParserState::Error;

  if(cbor_value_is_integer(value_iter)) {
    int val = 0;
    if(cbor_value_get_int(value_iter, &val) == CborNoError) {
      map_data->time.set(static_cast<double>(val));
    }
  }

  if(cbor_value_is_double(value_iter)) {
    double val = 0.0;
    if(cbor_value_get_double(value_iter, &val) == CborNoError) {
      map_data->time.set(val);
    }
  }

  if(cbor_value_is_float(value_iter)) {
    float val = 0.0;
    if(cbor_value_get_float(value_iter, &val) == CborNoError) {
      map_data->time.set(static_cast<double>(val));
    }
  }

  if(cbor_value_is_half_float(value_iter)) {
    uint16_t val = 0;
    if(cbor_value_get_half_float(value_iter, &val) == CborNoError) {
      map_data->time.set(static_cast<double>(convertCborHalfFloatToDouble(val)));
    }
  }

  if(cbor_value_advance(value_iter) == CborNoError) {
    next_state = MapParserState::MapKey;
  }

  return next_state;
}

ArduinoCloudThing::MapParserState ArduinoCloudThing::handle_LeaveMap(CborValue * map_iter, CborValue * value_iter, MapData const * const map_data) {
  MapParserState next_state = MapParserState::Error;

  /* Update the property containers depending on the parsed data */

  if(map_data->name.isSet())
  {
    /* Value (Integer/Float/Double/Half-Float) */
    if(map_data->val.isSet()) {
      ArduinoCloudProperty<int>   * int_property   = _property_cont.getPropertyInt  (map_data->name.get());
      ArduinoCloudProperty<float> * float_property = _property_cont.getPropertyFloat(map_data->name.get());

      if(int_property && int_property->isWriteableByCloud()) {
        int_property->writeByCloud(static_cast<int>(map_data->val.get())); /* Val is internally stored as float */
        int_property->execCallbackOnChange();
      }

      if(float_property && float_property->isWriteableByCloud()) {
        float_property->writeByCloud(map_data->val.get());
        float_property->execCallbackOnChange();
      }
    }

    /* Value (String) */
    if(map_data->str_val.isSet()) {
      ArduinoCloudProperty<String>* string_property = _property_cont.getPropertyString(map_data->name.get());
      if(string_property && string_property->isWriteableByCloud()) {
        string_property->writeByCloud(map_data->str_val.get());
        string_property->execCallbackOnChange();
      }
    }

    /* Value (bool) */
    if(map_data->bool_val.isSet()) {
      ArduinoCloudProperty<bool>* bool_property = _property_cont.getPropertyBool(map_data->name.get());
      if(bool_property && bool_property->isWriteableByCloud()) {
        bool_property->writeByCloud(map_data->bool_val.get());
        bool_property->execCallbackOnChange();
      }
    }
  }

  /* Transition into the next map if available, otherwise finish */

  if(cbor_value_leave_container(map_iter, value_iter) == CborNoError) {
    if(!cbor_value_at_end(map_iter)) {
      next_state = MapParserState::EnterMap;
    }
    else {
      next_state = MapParserState::Complete;
    }
  }

  return next_state;
}

void ArduinoCloudThing::resetMapData(MapData * map_data) {
  map_data->base_version.reset();
  map_data->base_name.reset   ();
  map_data->base_time.reset   ();
  map_data->name.reset        ();
  map_data->val.reset         ();
  map_data->str_val.reset     ();
  map_data->bool_val.reset    ();
  map_data->time.reset        ();
}

/* Source Idea from https://tools.ietf.org/html/rfc7049 : Page: 50 */
double ArduinoCloudThing::convertCborHalfFloatToDouble(uint16_t const half_val) {
  int exp = (half_val >> 10) & 0x1f;
  int mant = half_val & 0x3ff;
  double val;
  if (exp == 0) val = ldexp(mant, -24);
  else if (exp != 31) val = ldexp(mant + 1024, exp - 25);
  else val = mant == 0 ? INFINITY : NAN;
  return half_val & 0x8000 ? -val : val;
}
