/******************************************************************************
 * INCLUDE
 ******************************************************************************/

#include <Arduino.h>

#include <ArduinoCloudThing.h>

#include <math.h>

/******************************************************************************
 * PRIVATE FREE FUNCTIONS
 ******************************************************************************/

/* Source Idea from https://tools.ietf.org/html/rfc7049 : Page: 50 */
double convertCborHalfFloatToDouble(uint16_t const half_val) {
  int exp = (half_val >> 10) & 0x1f;
  int mant = half_val & 0x3ff;
  double val;
  if (exp == 0) val = ldexp(mant, -24);
  else if (exp != 31) val = ldexp(mant + 1024, exp - 25);
  else val = mant == 0 ? INFINITY : NAN;
  return half_val & 0x8000 ? -val : val;
}

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

void extractProperty(ArduinoCloudProperty<int> * int_property, CborValue * cbor_value)
{
  if (cbor_value->type == CborIntegerType) {
    int val = 0;
    cbor_value_get_int(cbor_value, &val);
    if(int_property->isWriteableByCloud()) {
      int_property->writeByCloud(val);
    }
  } else if (cbor_value->type == CborDoubleType) {
    double val = 0.0;
    cbor_value_get_double(cbor_value, &val);
    if(int_property->isWriteableByCloud()) {
      int_property->writeByCloud(static_cast<int>(val));
    }
  } else if (cbor_value->type == CborFloatType) {
    float val = 0.0f;
    cbor_value_get_float(cbor_value, &val);
    if(int_property->isWriteableByCloud()) {
      int_property->writeByCloud(static_cast<int>(val));
    }
  } else if (cbor_value->type == CborHalfFloatType) {
    uint16_t val = 0;
    cbor_value_get_half_float(cbor_value, &val);
    if(int_property->isWriteableByCloud()) {
      int_property->writeByCloud(static_cast<int>(convertCborHalfFloatToDouble(val)));
    }
  }
}

void extractProperty(ArduinoCloudProperty<float> * float_property, CborValue * cbor_value) {
  if (cbor_value->type == CborDoubleType) {
    double val = 0.0;
    cbor_value_get_double(cbor_value, &val);
    if(float_property->isWriteableByCloud()) {
      float_property->writeByCloud(static_cast<float>(val));
    }
  } else if (cbor_value->type == CborIntegerType) {
    int val = 0;
    cbor_value_get_int(cbor_value, &val);
    if(float_property->isWriteableByCloud()) {
      float_property->writeByCloud(static_cast<float>(val));
    }
  } else if (cbor_value->type == CborFloatType) {
    float val = 0.0f;
    cbor_value_get_float(cbor_value, &val);
    if(float_property->isWriteableByCloud()) {
      float_property->writeByCloud(val);
    }
  } else if (cbor_value->type == CborHalfFloatType) {
    uint16_t val = 0;
    cbor_value_get_half_float(cbor_value, &val);
    if(float_property->isWriteableByCloud()) {
      float_property->writeByCloud(static_cast<float>(convertCborHalfFloatToDouble(val)));
    }
  }
}

void extractProperty(ArduinoCloudProperty<bool> * bool_property, CborValue * cbor_value) {
  bool val = false;
  if(cbor_value_get_boolean(cbor_value, &val) == CborNoError) {
    if(bool_property->isWriteableByCloud()) {
      bool_property->writeByCloud(val);
    }
  }
}

void extractProperty(ArduinoCloudProperty<String> * string_property, CborValue * cbor_value) {
  char * val      = 0;
  size_t val_size = 0;
  if(cbor_value_dup_text_string(cbor_value, &val, &val_size, cbor_value) == CborNoError) {
    if(string_property->isWriteableByCloud()) {
      string_property->writeByCloud(static_cast<char *>(val));
    }
    free(val);
  }
}

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

  String            base_name,
                    property_name;
  CborIntegerMapKey property_value_type;

  enum class MapParserState {
    EnterMap,
    MapKey,
    BaseName,
    BaseTime,
    Time,
    PropertyName,
    PropertyType,
    PropertyValue,
    LeaveMap,
    Complete,
    Error
  };
  MapParserState current_state = MapParserState::EnterMap,
                 next_state;

  while(current_state != MapParserState::Complete)
  {
    switch(current_state) {
    /* MapParserState::EnterMap *****************************************/
    case MapParserState::EnterMap: {
      next_state = MapParserState::Error;

      if(cbor_value_get_type(&map_iter) == CborMapType) {
        if(cbor_value_enter_container(&map_iter, &value_iter) == CborNoError) {
          next_state = MapParserState::MapKey;
        }
      }
    }
    break;
    /* MapParserState::MapKey ****************************************/
    case MapParserState::MapKey: {
      next_state = MapParserState::Error;

      if(_cloud_protocol == CloudProtocol::V1) {
        if(cbor_value_is_text_string(&value_iter)) {
          char * val      = 0;
          size_t val_size = 0;
          if(cbor_value_dup_text_string(&value_iter, &val, &val_size, &value_iter) == CborNoError) {
            if     (strcmp(val, "n" ) == 0) { next_state = MapParserState::PropertyName; }
            else if(strcmp(val, "bn") == 0) { next_state = MapParserState::BaseName;     }
            else if(strcmp(val, "bt") == 0) { next_state = MapParserState::BaseTime;     }
            else if(strcmp(val, "t" ) == 0) { next_state = MapParserState::Time;         }
            free(val);
          }
        }
      }

      if(_cloud_protocol == CloudProtocol::V2) {
        if(cbor_value_is_integer(&value_iter)) {
          int val = 0;
          if(cbor_value_get_int(&value_iter, &val) == CborNoError) {
            if(cbor_value_advance(&value_iter) == CborNoError) {
              if     (val == static_cast<int>(CborIntegerMapKey::Name    )) { next_state = MapParserState::PropertyName; }
              else if(val == static_cast<int>(CborIntegerMapKey::BaseName)) { next_state = MapParserState::BaseName;     }
              else if(val == static_cast<int>(CborIntegerMapKey::BaseTime)) { next_state = MapParserState::BaseTime;     }
              else if(val == static_cast<int>(CborIntegerMapKey::Time    )) { next_state = MapParserState::Time;         }
            }
          }
        }
      }
    }
    break;
    /* MapParserState::BaseName *****************************************/
    case MapParserState::BaseName: {
      next_state = MapParserState::Error;

      if(cbor_value_is_text_string(&value_iter)) {
        char * val      = 0;
        size_t val_size = 0;
        if(cbor_value_dup_text_string(&value_iter, &val, &val_size, &value_iter) == CborNoError) {
          base_name = String(val);
          free(val);
          next_state = MapParserState::MapKey;
        }
      }
    }
    break;

    /* MapParserState::BaseTime *****************************************/
    case MapParserState::BaseTime: {
      /* TODO */
    }
    break;

    /* MapParserState::Time *****************************************/
    case MapParserState::Time : {
      /* TODO */
    }
    break;
    /* MapParserState::PropertyName *********************************************/
    case MapParserState::PropertyName: {
      next_state = MapParserState::Error;

      if(cbor_value_is_text_string(&value_iter)) {
        char * val      = 0;
        size_t val_size = 0;
        if(cbor_value_dup_text_string(&value_iter, &val, &val_size, &value_iter) == CborNoError) {
          property_name = String(val);
          free(val);
          next_state = MapParserState::PropertyType;
        }
      }
    }
    break;
    /* MapParserState::PropertyType ***************************************/
    case MapParserState::PropertyType: {
      next_state = MapParserState::Error;

      if(_cloud_protocol == CloudProtocol::V1) {
        if(cbor_value_is_text_string(&value_iter)) {
          char * val      = 0;
          size_t val_size = 0;
          if(cbor_value_dup_text_string(&value_iter, &val, &val_size, &value_iter) == CborNoError) {
            if     (strcmp(val, "v" ) == 0) property_value_type = CborIntegerMapKey::Value;
            else if(strcmp(val, "vs") == 0) property_value_type = CborIntegerMapKey::StringValue;
            else if(strcmp(val, "vb") == 0) property_value_type = CborIntegerMapKey::BooleanValue;
            free(val);
            next_state = MapParserState::PropertyValue;
          }
        }
      }

      if(_cloud_protocol == CloudProtocol::V2) {
        if(cbor_value_is_integer(&value_iter)) {
          int val = 0;
          if(cbor_value_get_int(&value_iter, &val) == CborNoError) {
            if     (val == static_cast<int>(CborIntegerMapKey::Value       )) property_value_type = CborIntegerMapKey::Value;
            else if(val == static_cast<int>(CborIntegerMapKey::StringValue )) property_value_type = CborIntegerMapKey::StringValue;
            else if(val == static_cast<int>(CborIntegerMapKey::BooleanValue)) property_value_type = CborIntegerMapKey::BooleanValue;
            if(cbor_value_advance(&value_iter) == CborNoError) {
              next_state = MapParserState::PropertyValue;
            }
          }
        }
      }
    }
    break;
    /* MapParserState::PropertyValue ********************************************/
    case MapParserState::PropertyValue: {
      next_state = MapParserState::Error;

      /* VALUE PROPERTY ******************************************************/
      if(property_value_type == CborIntegerMapKey::Value) {
        ArduinoCloudProperty<int>   * int_property   = _property_cont.getPropertyInt  (property_name);
        ArduinoCloudProperty<float> * float_property = _property_cont.getPropertyFloat(property_name);

        /* INT PROPERTY ******************************************************/
        if(int_property) {
          extractProperty(int_property, &value_iter);
          int_property->execCallbackOnChange();

          if(cbor_value_advance(&value_iter) == CborNoError) {
            next_state = MapParserState::LeaveMap;
          }
        }
        /* FLOAT PROPERTY ****************************************************/
        if(float_property) {
          extractProperty(float_property, &value_iter);
          float_property->execCallbackOnChange();

          if(cbor_value_advance(&value_iter) == CborNoError) {
            next_state = MapParserState::LeaveMap;
          }
        }
      }
      /* BOOL PROPERTY *******************************************************/
      if(property_value_type == CborIntegerMapKey::BooleanValue) {
        ArduinoCloudProperty<bool> * bool_property = _property_cont.getPropertyBool(property_name);
        if(bool_property) {
          extractProperty(bool_property, &value_iter);
          bool_property->execCallbackOnChange();

          if(cbor_value_advance(&value_iter) == CborNoError) {
            next_state = MapParserState::LeaveMap;
          }
        }
      }
      /* STRING PROPERTY *****************************************************/
      if(property_value_type == CborIntegerMapKey::StringValue) {
        ArduinoCloudProperty<String> * string_property = _property_cont.getPropertyString(property_name);
        if(string_property) {
          extractProperty(string_property, &value_iter);
          string_property->execCallbackOnChange();

          /* It is not necessary to advance it we have a string property
           * because extracting it automatically advances the iterator.
           */
          next_state = MapParserState::LeaveMap;
        }
      }
    }
    break;
    /* MapParserState::LeaveMap *****************************************/
    case MapParserState::LeaveMap: {
      next_state = MapParserState::Error;

      if(cbor_value_leave_container(&map_iter, &value_iter) == CborNoError) {
        if(!cbor_value_at_end(&map_iter)) {
          next_state = MapParserState::EnterMap;
        }
        else {
          next_state = MapParserState::Complete;
        }

      }
    }
    break;
    /* MapParserState::Complete ****************************************************/
    case MapParserState::Complete: {
      /* Nothing to do */
    }
    break;
    /* MapParserState::Error ****************************************************/
    case MapParserState::Error: {
      return;
    }
    break;
    }

    current_state = next_state;
  }
}
