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

#ifdef USE_ARDUINO_CLOUD
char MQTT_SERVER[] = "10.130.22.94";
int MQTT_PORT = 1883;
char GENERAL_TOPIC[] = "/main";
char MQTT_USER[] = "";
char MQTT_PASSWORD[] = "";
char LWT_TOPIC[] = "";
char LWT_MESSAGE[] = "";
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

/*
 * begin() should prepare the environment
 * connect 
 */

void ArduinoCloudThing::begin(Client &client) {
    this->client = new MQTTClient();
    this->client->onMessageAdvanced(ArduinoCloudThing::callback);
    this->client->begin(MQTT_SERVER, MQTT_PORT, client);
    this->client->setParent((void*)this);

    // using WiFi client and ECC508 connect to server
    while (!connect()) {
        Serial.println("Not connected");
        delay(500);
    }
}

bool ArduinoCloudThing::connect() {

#ifdef TESTING_PROTOCOL
    return true;
#endif

    if (client->connect(uuid, MQTT_USER, MQTT_PASSWORD) != 0) {
        // set status to ON
        status = ON;
        addProperty(status, READ);
        // subscribe to "general" topic
        client->subscribe(GENERAL_TOPIC);
        return true;
    }

    return false;
}

void ArduinoCloudThing::publish(CborArray& object) {

    bool retained = false;

    uint8_t data[1024];
    size_t size = object.encode(data, sizeof(data));

#ifdef TESTING_PROTOCOL
    decode(data, size);
#endif

#ifndef TESTING_PROTOCOL
    client->publish(GENERAL_TOPIC, (const char*)data, size);
#endif

    for (int i = 0; i < list.size(); i++) {
        ArduinoCloudPropertyGeneric *p = list.get(i);
        p->updateShadow();
    }
}

// Reconnect to the mqtt broker
int ArduinoCloudThing::poll() {

#ifndef TESTING_PROTOCOL
    if (!client->connected()){
        if (!connect()){
            return 0;
        }
    }
#endif

    // check if backing storage and cloud has diverged
    int diff = 0;

    diff = checkNewData();
    if (diff > 0) {
        CborBuffer buffer(1024);
        CborArray object = CborArray(buffer);
        compress(object, buffer);
        publish(object);
    }

#if defined(DEBUG_MEMORY) && defined(ARDUINO_ARCH_SAMD)
    PrintFreeRam();
#endif

    return diff;
}

void ArduinoCloudThing::compress(CborArray& object, CborBuffer& buffer) {

    for (int i = 0; i < list.size(); i++) {
        ArduinoCloudPropertyGeneric *p = list.get(i);
        if (p->shouldBeUpdated()) {
            CborObject child = CborObject(buffer);
            p->append(child);
            CborVariant variant = CborVariant(buffer, child);
            object.add(variant);
        }
    }
}

int ArduinoCloudThing::checkNewData() {
    int counter = 0;
    for (int i = 0; i < list.size(); i++) {
        ArduinoCloudPropertyGeneric *p = list.get(i);
        if (p->shouldBeUpdated()) {
            counter++;
        }
    }
    return counter;
}

bool ArduinoCloudThing::exists(String &name) {
    for (int i = 0; i < list.size(); i++) {
        ArduinoCloudPropertyGeneric *p = list.get(i);
        if (p->getName() == name) {
            return true;
        }
    }
    return false;
}

ArduinoCloudPropertyGeneric& ArduinoCloudThing::addPropertyReal(int& property, String name, permissionType permission) {
    if (exists(name)) {
        return *(reinterpret_cast<ArduinoCloudPropertyGeneric*>(NULL));
    }
    ArduinoCloudProperty<int> *thing = new ArduinoCloudProperty<int>(property, name, permission);
    list.add(thing);
    return *(reinterpret_cast<ArduinoCloudPropertyGeneric*>(thing));
}

ArduinoCloudPropertyGeneric& ArduinoCloudThing::addPropertyReal(bool& property, String name, permissionType permission) {
    if (exists(name)) {
        return *(reinterpret_cast<ArduinoCloudPropertyGeneric*>(NULL));
    }
    ArduinoCloudProperty<bool> *thing = new ArduinoCloudProperty<bool>(property, name, permission);
    list.add(thing);
    return *(reinterpret_cast<ArduinoCloudPropertyGeneric*>(thing));
}

ArduinoCloudPropertyGeneric& ArduinoCloudThing::addPropertyReal(float& property, String name, permissionType permission) {
    if (exists(name)) {
        return *(reinterpret_cast<ArduinoCloudPropertyGeneric*>(NULL));
    }
    ArduinoCloudProperty<float> *thing = new ArduinoCloudProperty<float>(property, name, permission);
    list.add(thing);
    return *(reinterpret_cast<ArduinoCloudPropertyGeneric*>(thing));
}

void ArduinoCloudThing::callback(MQTTClient *client, char topic[], char bytes[], int length) {
    reinterpret_cast<ArduinoCloudThing*>(client->getParent())->decode((uint8_t *)bytes, length);
}

void ArduinoCloudThing::decode(uint8_t * payload, size_t length) {
    CborBuffer buffer(200);
    CborVariant total = buffer.decode(payload, length);

    CborArray array = total.asArray();

    for (int i=0; ;i++) {
    CborVariant variant = array.get(i);

        if (!variant.isValid()) {
            break;
        }


        CborObject object = variant.asObject();

        String name = "";
        if (object.get("n").isValid()) {
            name = object.get("n").asString();
            // search for the property with the same name
            for (int idx = 0; idx < list.size(); idx++) {
                ArduinoCloudPropertyGeneric *p = list.get(idx);
                if (p->getName() == name) {
                    currentListIndex = idx;
                    break;
                }
                if (idx == list.size()) {
                    currentListIndex = -1;
                }
            }
        }

        if (object.get("t").isValid()) {
            int tag = object.get("t").asInteger();
            if (name != "") {
                list.get(currentListIndex)->setTag(tag);
            } else {
                for (int idx = 0; idx < list.size(); idx++) {
                    ArduinoCloudPropertyGeneric *p = list.get(idx);
                    if (p->getTag() == tag) {
                        // if name == "" associate name and tag, otherwise set current list index
                        currentListIndex = idx;
                        break;
                    }
                    if (idx == list.size()) {
                        Serial.println("Property not found, skipping");
                        currentListIndex = -1;
                    }
                }
            }
        }

        if (object.get("i").isValid()) {
            int value_i = object.get("i").asInteger();
            reinterpret_cast<ArduinoCloudProperty<int>*>(list.get(currentListIndex))->write(value_i);
        }

        if (object.get("b").isValid()) {
            bool value_b = object.get("b").asInteger();
            reinterpret_cast<ArduinoCloudProperty<bool>*>(list.get(currentListIndex))->write(value_b);
        }
/*
        if (object.get("f").isValid()) {
            float value_f = object.get("f").asFloat();
            reinterpret_cast<ArduinoCloudProperty<bool>*>(list.get(currentListIndex))->write(value_f);
        }
*/
        if (object.get("s").isValid()) {
            String value_s = object.get("s").asString();
            reinterpret_cast<ArduinoCloudProperty<String>*>(list.get(currentListIndex))->write(value_s);
        }

        if (object.get("p").isValid()) {
            permissionType value_p = (permissionType)object.get("p").asInteger();
            list.get(currentListIndex)->setPermission(value_p);
        }

        if (list.get(currentListIndex)->newData()) {
            // call onUpdate()
            if (list.get(currentListIndex)->callback != NULL) {
                list.get(currentListIndex)->callback();
            }
        }
    }
}