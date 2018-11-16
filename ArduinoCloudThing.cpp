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

    CborError err;
    CborParser parser;
    CborValue recursedMap, propValue, dataArray;
    String propName;
    Type propType;

    err = cbor_parser_init(payload, length, 0, &parser, &dataArray);
    if(err) {
        //Serial.println("Error in the parser creation.");
        //Serial.println(cbor_error_string(err));
        return;
    }

    // parse cbor data only if a cbor array is received.
    if(dataArray.type != CborArrayType)
        return;

    // main loop through the cbor array elements
    while (!cbor_value_at_end(&dataArray)) {

        // parse cbor object
        cbor_value_enter_container(&dataArray, &recursedMap);

        if (cbor_value_get_type(&recursedMap) != CborMapType) {
            // stop the decode when 1st item thai is not a cbor map is found.
            err = cbor_value_advance(&dataArray);
            // avoid infinite loop if it is not possible to advance to the next array value
            if (err != CborNoError) {
                break;
            }
            // go to the next element
            continue;

        } else {
            while (!cbor_value_at_end(&recursedMap)) {

                // if the current element is not a cbor object as expected, skip it and go ahead.
                if(cbor_value_get_type(&recursedMap) != CborMapType) {

                    err = cbor_value_advance(&recursedMap);
                    if (err != CborNoError) {
                        break;
                    }
                    continue;
                }

                CborValue name;
                // chechk for the if the a property has a name, if yes Cbor value name will properly updated
                cbor_value_map_find_value(&recursedMap, "n", &name);

                // check if a property has a name, of string type, if not do nothing and skip curtrent property
                if (name.type != CborTextStringType) {
                    err = cbor_value_advance(&recursedMap);
                    // problem to advance to the next array object
                    if (err != CborNoError)
                        break;

                    continue;
                }

                // get the property name from cbor map as char* string
                char *nameVal; size_t nameValSize;
                err = cbor_value_dup_text_string(&name, &nameVal, &nameValSize, NULL);
                if (err) {
                    break; // couldn't get the value of the field
                }
                // get the name of the received property as String object
                propName = String(nameVal);
                // used to avoid memory leaks (cbor_value_dup_text_string automatically perform a malloc)
                free(nameVal);

                // Search for the device property with that name
                ArduinoCloudProperty<bool>   * bool_property   = _property_cont.getPropertyBool  (propName);
                ArduinoCloudProperty<int>    * int_property    = _property_cont.getPropertyInt   (propName);
                ArduinoCloudProperty<float>  * float_property  = _property_cont.getPropertyFloat (propName);
                ArduinoCloudProperty<String> * string_property = _property_cont.getPropertyString(propName);

                // If property does not exist, skip it and do nothing.
                if((bool_property == 0) && (int_property == 0) && (float_property == 0) && (string_property == 0))
                {
                  cbor_value_advance(&recursedMap);
                  continue;
                }

                if     (bool_property   != 0) propType = Type::Bool;
                else if(int_property    != 0) propType = Type::Int;
                else if(float_property  != 0) propType = Type::Float;
                else if(string_property != 0) propType = Type::String;

                if (propType == Type::Float && !cbor_value_map_find_value(&recursedMap, "v", &propValue)) {
                    if (propValue.type == CborDoubleType) {
                        double val;
                        // get the value of the property as a double
                        cbor_value_get_double(&propValue, &val);
                        if(float_property->isWriteableByCloud()) {
                          float_property->writeByCloud(static_cast<float>(val));
                        }
                    } else if (propValue.type == CborIntegerType) {
                        int val;
                        cbor_value_get_int(&propValue, &val);
                        if(float_property->isWriteableByCloud()) {
                          float_property->writeByCloud(static_cast<float>(val));
                        }
                    } else if (propValue.type == CborFloatType) {
                        float val;
                        cbor_value_get_float(&propValue, &val);
                        if(float_property->isWriteableByCloud()) {
                          float_property->writeByCloud(val);
                        }
                    } else if (propValue.type == CborHalfFloatType) {
                        uint16_t val;
                        cbor_value_get_half_float(&propValue, &val);
                        if(float_property->isWriteableByCloud()) {
                          float_property->writeByCloud(static_cast<float>(convertCborHalfFloatToDouble(val)));
                        }
                    }
                    float_property->execCallbackOnChange();
                } else if (propType == Type::Int && !cbor_value_map_find_value(&recursedMap, "v", &propValue)) {
                    // if no key proper key was found, do nothing
                    if (propValue.type == CborIntegerType) {
                        int val;
                        cbor_value_get_int(&propValue, &val);
                        if(int_property->isWriteableByCloud()) {
                          int_property->writeByCloud(val);
                        }
                    } else if (propValue.type == CborDoubleType) {
                        // If a double value is received, a cast to int is performed(so it is still accepted)
                        double val;
                        cbor_value_get_double(&propValue, &val);
                        if(int_property->isWriteableByCloud()) {
                          int_property->writeByCloud(static_cast<int>(val));
                        }
                    } else if (propValue.type == CborFloatType) {
                        float val;
                        cbor_value_get_float(&propValue, &val);
                        if(int_property->isWriteableByCloud()) {
                          int_property->writeByCloud(static_cast<int>(val));
                        }
                    } else if (propValue.type == CborHalfFloatType) {
                        uint16_t val;
                        cbor_value_get_half_float(&propValue, &val);
                        if(int_property->isWriteableByCloud()) {
                          int_property->writeByCloud(static_cast<int>(convertCborHalfFloatToDouble(val)));
                        }
                    }
                    int_property->execCallbackOnChange();
                } else if (propType == Type::Bool && !cbor_value_map_find_value(&recursedMap, "vb", &propValue)) {
                    if (propValue.type == CborBooleanType) {
                        bool val;
                        cbor_value_get_boolean(&propValue, &val);
                        if(bool_property->isWriteableByCloud()) {
                          bool_property->writeByCloud(val);
                          bool_property->execCallbackOnChange();
                        }
                    }
                } else if (propType == Type::String && !cbor_value_map_find_value(&recursedMap, "vs", &propValue)){
                    if (propValue.type == CborTextStringType) {
                        char *val; size_t valSize;
                        err = cbor_value_dup_text_string(&propValue, &val, &valSize, &propValue);
                        if(string_property->isWriteableByCloud()) {
                          string_property->writeByCloud(static_cast<char *>(val));
                          string_property->execCallbackOnChange();
                        }
                        free(val);
                    }
                }

                // Continue to scan the cbor map
                err = cbor_value_advance(&recursedMap);
                if (err != CborNoError) {
                    break;
                }
            }
        }

        if (err != CborNoError)
            break;
        // Leave the current cbor object, and advance to the next one
        cbor_value_leave_container(&dataArray, &recursedMap);

    }
}
