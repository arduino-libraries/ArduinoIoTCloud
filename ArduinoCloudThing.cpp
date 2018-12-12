/******************************************************************************
 * INCLUDE
 ******************************************************************************/

//#include <Arduino.h>

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

/******************************************************************************
 * CTOR/DTOR
 ******************************************************************************/

ArduinoCloudThing::ArduinoCloudThing() {
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

        _property_cont.appendChangedProperties(&arrayEncoder);

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
  CborValue  data_array, recursed_map;

  if(cbor_parser_init(payload, length, 0, &parser, &data_array) != CborNoError)
    return;

  if(data_array.type != CborArrayType)
    return;

  if(cbor_value_enter_container(&data_array, &recursed_map) != CborNoError)
    return;

  while(!cbor_value_at_end(&data_array)) {

    String            property_name;
    CborIntegerMapKey property_value_type;
    CborValue         recursed_map_save;

    enum class ParserState {
      EnterPropertyMap,
      PropertyNameLabel,
      PropertyName,
      PropertyValueLabel,
      PropertyValue,
      LeavePropertyMap,
      Error
    };
    ParserState current_state = ParserState::EnterPropertyMap,
                next_state    = ParserState::EnterPropertyMap;

    while (!cbor_value_at_end(&recursed_map)) {

      do
      {
        current_state = next_state;

        switch(current_state) {
        /* ParserState::EnterPropertyMap *****************************************/
        case ParserState::EnterPropertyMap: {
          next_state = ParserState::Error;

          if(cbor_value_get_type(&recursed_map) == CborMapType) {
            recursed_map_save = recursed_map;
            if(cbor_value_enter_container(&recursed_map_save, &recursed_map) == CborNoError) {
              next_state = ParserState::PropertyNameLabel;
            }
          }
        }
        break;
        /* ParserState::PropertyNameLabel ****************************************/
        case ParserState::PropertyNameLabel: {
          next_state = ParserState::Error;

          if(cbor_value_is_text_string(&recursed_map)) {
            char * val      = 0;
            size_t val_size = 0;
            if(cbor_value_dup_text_string(&recursed_map, &val, &val_size, &recursed_map) == CborNoError) {
              if(strcmp(val, "n") == 0) {
                next_state = ParserState::PropertyName;
              }
              free(val);
            }
          }
        }
        break;
        /* ParserState::PropertyName *********************************************/
        case ParserState::PropertyName: {
          next_state = ParserState::Error;

          if(cbor_value_is_text_string(&recursed_map)) {
            char * val      = 0;
            size_t val_size = 0;
            if(cbor_value_dup_text_string(&recursed_map, &val, &val_size, &recursed_map) == CborNoError) {
              property_name = String(val);
              free(val);
              next_state = ParserState::PropertyValueLabel;
            }
          }
        }
        break;
        /* ParserState::PropertyValueLabel ***************************************/
        case ParserState::PropertyValueLabel: {
          next_state = ParserState::Error;

          if(cbor_value_is_text_string(&recursed_map)) {
            char * val      = 0;
            size_t val_size = 0;
            if(cbor_value_dup_text_string(&recursed_map, &val, &val_size, &recursed_map) == CborNoError) {
                   if(strcmp(val, "v" ) == 0) property_value_type = CborIntegerMapKey::Value;
              else if(strcmp(val, "vs") == 0) property_value_type = CborIntegerMapKey::StringValue;
              else if(strcmp(val, "vb") == 0) property_value_type = CborIntegerMapKey::BooleanValue;
              free(val);
              next_state = ParserState::PropertyValue;
            }
          }
        }
        break;
        /* ParserState::PropertyValue ********************************************/
        case ParserState::PropertyValue: {
          next_state = ParserState::Error;

          /* VALUE PROPERTY ******************************************************/
          if(property_value_type == CborIntegerMapKey::Value) {
            ArduinoCloudProperty<int>   * int_property   = _property_cont.getPropertyInt  (property_name);
            ArduinoCloudProperty<float> * float_property = _property_cont.getPropertyFloat(property_name);

            /* INT PROPERTY ******************************************************/
            if(int_property) {
              if (recursed_map.type == CborIntegerType) {
                  int val = 0;
                  cbor_value_get_int(&recursed_map, &val);
                  if(int_property->isWriteableByCloud()) {
                    int_property->writeByCloud(val);
                  }
              } else if (recursed_map.type == CborDoubleType) {
                  double val = 0.0;
                  cbor_value_get_double(&recursed_map, &val);
                  if(int_property->isWriteableByCloud()) {
                    int_property->writeByCloud(static_cast<int>(val));
                  }
              } else if (recursed_map.type == CborFloatType) {
                  float val = 0.0f;
                  cbor_value_get_float(&recursed_map, &val);
                  if(int_property->isWriteableByCloud()) {
                    int_property->writeByCloud(static_cast<int>(val));
                  }
              } else if (recursed_map.type == CborHalfFloatType) {
                  uint16_t val = 0;
                  cbor_value_get_half_float(&recursed_map, &val);
                  if(int_property->isWriteableByCloud()) {
                    int_property->writeByCloud(static_cast<int>(convertCborHalfFloatToDouble(val)));
                  }
              }
              int_property->execCallbackOnChange();
            }

            /* FLOAT PROPERTY ****************************************************/
            if(float_property) {
              if (recursed_map.type == CborDoubleType) {
                  double val = 0.0;
                  cbor_value_get_double(&recursed_map, &val);
                  if(float_property->isWriteableByCloud()) {
                    float_property->writeByCloud(static_cast<float>(val));
                  }
              } else if (recursed_map.type == CborIntegerType) {
                  int val = 0;
                  cbor_value_get_int(&recursed_map, &val);
                  if(float_property->isWriteableByCloud()) {
                    float_property->writeByCloud(static_cast<float>(val));
                  }
              } else if (recursed_map.type == CborFloatType) {
                  float val = 0.0f;
                  cbor_value_get_float(&recursed_map, &val);
                  if(float_property->isWriteableByCloud()) {
                    float_property->writeByCloud(val);
                  }
              } else if (recursed_map.type == CborHalfFloatType) {
                  uint16_t val = 0;
                  cbor_value_get_half_float(&recursed_map, &val);
                  if(float_property->isWriteableByCloud()) {
                    float_property->writeByCloud(static_cast<float>(convertCborHalfFloatToDouble(val)));
                  }
              }
              float_property->execCallbackOnChange();
            }
          }

          /* BOOL PROPERTY *******************************************************/
          if(property_value_type == CborIntegerMapKey::BooleanValue) {
            ArduinoCloudProperty<bool> * bool_property = _property_cont.getPropertyBool(property_name);
            if(bool_property)
            {
              bool val = false;
              if(cbor_value_get_boolean(&recursed_map, &val) == CborNoError) {
                if(bool_property->isWriteableByCloud()) {
                  bool_property->writeByCloud(val);
                  bool_property->execCallbackOnChange();
                }
              }
            }
          }
          /* STRING PROPERTY *****************************************************/
          if(property_value_type == CborIntegerMapKey::StringValue) {
            ArduinoCloudProperty<String> * string_property = _property_cont.getPropertyString(property_name);
            if(string_property)
            {
              char * val      = 0;
              size_t val_size = 0;
              if(cbor_value_dup_text_string(&recursed_map, &val, &val_size, &recursed_map) == CborNoError) {
                if(string_property->isWriteableByCloud()) {
                  string_property->writeByCloud(static_cast<char *>(val));
                  string_property->execCallbackOnChange();
                }
                free(val);
              }
            }
          }


          if(cbor_value_advance(&recursed_map) == CborNoError) {
            next_state = ParserState::LeavePropertyMap;
          }

          /* TODO */


        }
        break;
        /* ParserState::LeavePropertyMap *****************************************/
        case ParserState::LeavePropertyMap: {
          next_state = ParserState::Error;

          if(cbor_value_leave_container(&recursed_map_save, &recursed_map) == CborNoError) {
            next_state = ParserState::EnterPropertyMap;
          }
        }
        break;
        /* ParserState::Error ****************************************************/
        case ParserState::Error: {
          return;
        }
        break;
        }
      } while(current_state != next_state);
    }
  }
}
