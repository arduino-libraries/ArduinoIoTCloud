#include <Arduino.h>
#include <ArduinoCloudThing.h>

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

ArduinoCloudThing::ArduinoCloudThing() {
#ifdef ARDUINO_ARCH_SAMD
    #define SERIAL_NUMBER_WORD_0    *(volatile uint32_t*)(0x0080A00C)
    #define SERIAL_NUMBER_WORD_1    *(volatile uint32_t*)(0x0080A040)
    #define SERIAL_NUMBER_WORD_2    *(volatile uint32_t*)(0x0080A044)
    #define SERIAL_NUMBER_WORD_3    *(volatile uint32_t*)(0x0080A048)

    utox8(SERIAL_NUMBER_WORD_0, &uuid[0]);
    utox8(SERIAL_NUMBER_WORD_1, &uuid[8]);
    utox8(SERIAL_NUMBER_WORD_2, &uuid[16]);
    utox8(SERIAL_NUMBER_WORD_3, &uuid[24]);
    uuid[32] = '\0';
#endif
}


void ArduinoCloudThing::begin() {

    status = ON;
    addPropertyReal(status, "status").readOnly();
}

int ArduinoCloudThing::poll(uint8_t* data, size_t size) {

    // check if backing storage and cloud has diverged
    int diff = 0;

    diff = checkNewData();
    if (diff > 0) {
        CborError err;
        CborEncoder encoder, arrayEncoder;
         cbor_encoder_init(&encoder, data, size, 0);
        // create a cbor array containing the property that should be updated.
        err = cbor_encoder_create_array(&encoder, &arrayEncoder, CborIndefiniteLength);
        if (err) {
            Serial.println(cbor_error_string(err));
            return -1;
        }
         for (int i = 0; i < list.size(); i++) {
            ArduinoCloudPropertyGeneric *p = list.get(i);
            // If a property should be updated and has read permission from the Cloud point of view
            if (p->shouldBeUpdated() && p->canRead()) {
                // create a cbor object for the property and automatically add it into array
                p->append(&arrayEncoder);
            }
        }

        err = cbor_encoder_close_container(&encoder, &arrayEncoder);
        // update properties shadow values, in order to check if variable has changed since last publish

        for (int i = 0; i < list.size(); i++) {
            ArduinoCloudPropertyGeneric *p = list.get(i);
            p->updateShadow();
        }
         // return the number of byte of the CBOR encoded array
        return cbor_encoder_get_buffer_size(&encoder, data);
    }

#if defined(DEBUG_MEMORY) && defined(ARDUINO_ARCH_SAMD)
    PrintFreeRam();
#endif
     // If nothing has to be sent, return diff, that is 0 in this case
    return diff;
}

// It return the index of the property, inside the local array, with the name passed as parameter. (-1 if it does not exist.)
int ArduinoCloudThing::findPropertyByName(String &name) {
    for (int i = 0; i < list.size(); i++) {
        ArduinoCloudPropertyGeneric *p = list.get(i);
        // Check the property existance just comparing its name with existent ones
        if (p->getName() == name) {
            return i;
        }
    }
    return -1;
}

int ArduinoCloudThing::checkNewData() {
    int counter = 0;
    for (int i = 0; i < list.size(); i++) {
        ArduinoCloudPropertyGeneric *p = list.get(i);
        if (p->shouldBeUpdated() && p->canRead()) {
            counter++;
        }
    }
    return counter;
}

ArduinoCloudPropertyGeneric* ArduinoCloudThing::exists(String &name) {
    for (int i = 0; i < list.size(); i++) {
        ArduinoCloudPropertyGeneric *p = list.get(i);
        if (p->getName() == name) {
            return p;
        }
    }
    return NULL;
}

ArduinoCloudPropertyGeneric& ArduinoCloudThing::addPropertyReal(int& property, String name) {
    if (ArduinoCloudPropertyGeneric* p = exists(name)) {
        return *p;
    }
    ArduinoCloudProperty<int> *thing = new ArduinoCloudProperty<int>(property, name);
    list.add(thing);
    thing->shadow_property = -1;
    return *(reinterpret_cast<ArduinoCloudPropertyGeneric*>(thing));
}

ArduinoCloudPropertyGeneric& ArduinoCloudThing::addPropertyReal(bool& property, String name) {
    if (ArduinoCloudPropertyGeneric* p = exists(name)) {
        return *p;
    }
    ArduinoCloudProperty<bool> *thing = new ArduinoCloudProperty<bool>(property, name);
    list.add(thing);
    thing->shadow_property = !property;
    return *(reinterpret_cast<ArduinoCloudPropertyGeneric*>(thing));
}

ArduinoCloudPropertyGeneric& ArduinoCloudThing::addPropertyReal(float& property, String name) {
    if (ArduinoCloudPropertyGeneric* p = exists(name)) {
        return *p;
    }
    ArduinoCloudProperty<float> *thing = new ArduinoCloudProperty<float>(property, name);
    list.add(thing);
    thing->shadow_property = property - 1.0f;
    return *(reinterpret_cast<ArduinoCloudPropertyGeneric*>(thing));
}

ArduinoCloudPropertyGeneric& ArduinoCloudThing::addPropertyReal(String& property, String name) {
    if (ArduinoCloudPropertyGeneric* p = exists(name)) {
        return *p;
    }
    ArduinoCloudProperty<String> *thing = new ArduinoCloudProperty<String>(property, name);
    list.add(thing);
    thing->shadow_property = "";
    return *(reinterpret_cast<ArduinoCloudPropertyGeneric*>(thing));
}

void ArduinoCloudThing::decode(uint8_t * payload, size_t length) {
    CborError err;
    CborParser parser;
    CborValue recursedMap, propValue, dataArray;
    int propId; String propType, propName;

    err = cbor_parser_init(payload, length, 0, &parser, &dataArray);
    if (err) {
        return;
    }

    // parse cbor data only if a cbor array is received.
    if (dataArray.type != CborArrayType)
        return;

    // main loop through the cbor array elements
    while (!cbor_value_at_end(&dataArray)) {
         // parse cbor object
        cbor_value_enter_container(&dataArray, &recursedMap);
        CborType type = cbor_value_get_type(&recursedMap);
        if (type != CborMapType) {
            // stop the decode when 1st item thai is not a cbor map is found.
            cbor_value_advance(&dataArray);
            continue;
        } else {

            while (!cbor_value_at_end(&recursedMap)) {
                 // if the current element is not a cbor object as expected, skip it and go ahead.
                if (cbor_value_get_type(&recursedMap) != CborMapType) {
                    cbor_value_advance(&recursedMap);
                    continue;
                }

                CborValue name;
                // chechk for the if the a property has a name, if yes Cbor value name will properly updated
                cbor_value_map_find_value(&recursedMap, "n", &name);
                 // check if a property has a name, of string type, if not do nothin and skip curtrent property
                if (name.type != CborTextStringType) {
                    cbor_value_advance(&recursedMap);
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
                 // Search for the index of the device property with that name
                propId = findPropertyByName(propName);
                // If property does not exist, skip it and do nothing.
                if (propId < 0) {
                    cbor_value_advance(&recursedMap);
                    continue;
                }

                ArduinoCloudPropertyGeneric* property = list.get(propId);
                // Check for the property type, write method internally check for the permission

                cbor_value_map_find_value(&recursedMap, "v", &propValue);

                if (propValue.type == CborDoubleType) {
                    double val;
                    // get the value of the property as a double
                    cbor_value_get_double(&propValue, &val);
                    reinterpret_cast<ArduinoCloudProperty<float>*>(property)->write((float)val);
                }
                // if no key proper key was found, do nothing
                if (propValue.type == CborIntegerType) {
                    int val;
                    cbor_value_get_int(&propValue, &val);
                    reinterpret_cast<ArduinoCloudProperty<int>*>(property)->write(val);
                }
                if (propValue.type == CborBooleanType) {
                    bool val;
                    cbor_value_get_boolean(&propValue, &val);
                    reinterpret_cast<ArduinoCloudProperty<bool>*>(property)->write(val);
                }
                if (propValue.type == CborTextStringType) {
                    char *val; size_t valSize;
                    err = cbor_value_dup_text_string(&propValue, &val, &valSize, &propValue);
                    // Char* string transformed into array
                    reinterpret_cast<ArduinoCloudProperty<String>*>(property)->write(String((char*)val));
                    free(val);
                }
                // If the property has been changed call its callback
                if (property->newData()) {
                    if (property->callback != NULL) {
                        property->callback();
                    }
                }
                // Continue to scan the cbor map
                cbor_value_advance(&recursedMap);
            }
        }
        // Leave the current cbor object, and advance to the next one
        err = cbor_value_leave_container(&dataArray, &recursedMap);
    }
}