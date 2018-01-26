#include <Arduino.h>
#include <ArduinoCloudThing.h>

#ifdef DEBUG_MEMORY
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

void ArduinoCloudThing::publish(CborObject& object) {

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
        CborObject object = CborObject(buffer);
        compress(object, buffer);
        publish(object);
    }

#ifdef DEBUG_MEMORY
    PrintFreeRam();
#endif

    return diff;
}

void ArduinoCloudThing::compress(CborObject& object, CborBuffer& buffer) {

    CborArray array = CborArray(buffer);

    for (int i = 0; i < list.size(); i++) {
        ArduinoCloudPropertyGeneric *p = list.get(i);
        if (p->newData()) {
            CborObject child = CborObject(buffer);
            p->append(child);
            CborVariant variant = CborVariant(buffer, child);
            array.add(variant);
        }
    }
    object.set("a", array);
}

int ArduinoCloudThing::checkNewData() {
    int counter = 0;
    for (int i = 0; i < list.size(); i++) {
        ArduinoCloudPropertyGeneric *p = list.get(i);
        if (p->newData()) {
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

void ArduinoCloudThing::addPropertyReal(int& property, String name, permissionType permission) {
    if (exists(name)) {
        return;
    }
    ArduinoCloudProperty<int> *thing = new ArduinoCloudProperty<int>(property, name, permission);
    list.add(thing);
}

void ArduinoCloudThing::addPropertyReal(bool& property, String name, permissionType permission) {
    if (exists(name)) {
        return;
    }
    ArduinoCloudProperty<bool> *thing = new ArduinoCloudProperty<bool>(property, name, permission);
    list.add(thing);
}

void ArduinoCloudThing::addPropertyReal(float& property, String name, permissionType permission) {
    if (exists(name)) {
        return;
    }
    ArduinoCloudProperty<float> *thing = new ArduinoCloudProperty<float>(property, name, permission);
    list.add(thing);
}

void ArduinoCloudThing::callback(MQTTClient *client, char topic[], char bytes[], int length) {
    reinterpret_cast<ArduinoCloudThing*>(client->getParent())->decode((uint8_t *)bytes, length);
}

void ArduinoCloudThing::decode(uint8_t * payload, size_t length) {
    /*
    CborBuffer buffer(200);
    CborVariant variant = buffer.decode(payload, length);
    CborArray array = variant.asArray();

    CborVariant obj = array.get(0);
    if (!obj.isValid()) {
        return;
    }
    if (obj.isString()) {

    }
    */
}

/*

void CborPropertyListener::OnInteger(int32_t value) {
    if (currentListIndex < 0) {
        return;
    }
    reinterpret_cast<ArduinoCloudProperty<int>*>(list->get(currentListIndex))->write(value);
}

void CborPropertyListener::OnBytes(unsigned char *data, unsigned int size) {
    printf("bytes with size: %d", size);
}

void CborPropertyListener::OnString(String &str) {
    // if tag arrived, search a string with the same name in the list
    if (newElement == true) {
        newElement = false;
        for (int i = 0; i < list->size(); i++) {
            ArduinoCloudPropertyGeneric *p = list->get(i);
            if (p->getName() == str) {
                currentListIndex = i;
                break;
            }
            if (i == list->size()) {
                Serial.println("Property not found, skipping");
                currentListIndex = -1;
            }
        }
    } else {
        if (currentListIndex < 0) {
            return;
        }
        reinterpret_cast<ArduinoCloudProperty<String>*>(list->get(currentListIndex))->write(str);
    }
}

void CborPropertyListener::OnArray(unsigned int size) {

    // prepare for new properties to arrive
    if (justStarted == true) {
        list_size = size;
        justStarted = false;
    }
}

void CborPropertyListener::OnMap(unsigned int size) {
}

void CborPropertyListener::OnTag(uint32_t tag) {
     newElement = true;
     list_size--;
     if (list_size < 0) {
        Serial.println("problem, we got more properties than advertised");
     }
}

void CborPropertyListener::OnSpecial(uint32_t code) {

    if (currentListIndex < 0) {
        return;
    }
    if (list->get(currentListIndex)->getPermission() != code) {
        Serial.println("permission changed, updating");
        list->get(currentListIndex)->setPermission((permissionType)code);
    }
}

void CborPropertyListener::OnError(const char *error) {
}

*/