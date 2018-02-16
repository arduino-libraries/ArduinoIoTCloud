#ifndef ArduinoCloudThing_h
#define ArduinoCloudThing_h

#include <Client.h>
#include <Stream.h>
#include "lib/MQTT/src/MQTTClient.h"
#include "lib/LinkedList/LinkedList.h"
#include "lib/ArduinoCbor/src/ArduinoCbor.h"

#ifndef MQTT_BUFFER_SIZE
#define MQTT_BUFFER_SIZE 256
#endif

//#define TESTING_PROTOCOL
#define DEBUG_MEMORY
#define USE_ARDUINO_CLOUD

//#define MQTTCLIENT_QOS1 0
//#define MQTTCLIENT_QOS2 0

enum permissionType {
    READ    = 0b01,
    WRITE   = 0b10,
    READWRITE = READ|WRITE,
};

enum boolStatus {
    ON,
    OFF,
};

#define ON_CHANGE   -1

enum times {
    SECONDS = 1,
    MINUTES = 60,
    HOURS = 3600,
    DAYS = 86400,
};

class ArduinoCloudPropertyGeneric
{
public:
    virtual void append(CborObject& object) = 0;
    virtual String& getName() = 0;
    virtual void setName(String _name) = 0;
    virtual void setTag(int _tag) = 0;
    virtual int getTag() = 0;
    virtual void setPermission(permissionType _permission) = 0;
    virtual permissionType getPermission() = 0;
    virtual bool newData() = 0;
    virtual bool shouldBeUpdated() = 0;
    virtual void updateShadow() = 0;
    virtual ArduinoCloudPropertyGeneric& onUpdate(void(*fn)(void)) = 0;
    virtual ArduinoCloudPropertyGeneric& publishEvery(long seconds) = 0;
    void(*callback)(void) = NULL;
};

template <typename T>
class ArduinoCloudProperty : public ArduinoCloudPropertyGeneric
{
public:
    ArduinoCloudProperty(T& _property, String _name, permissionType _permission) :
        property(_property), name(_name), permission(_permission) {}

    bool write(T value) {
        if (permission != READ) {
            property = value;
            return true;
        }
        return false;
    }

    void updateShadow() {
        shadow_property = property;
    }

    T read() {
        if (permission != WRITE) {
            return property;
        }
    }

    String& getName() {
        return name;
    }

    void setName(String _name) {
        name = _name;
    }

    void setTag(int _tag) {
        tag = _tag;
    }

    int getTag() {
        return tag;
    }

    void setPermission(permissionType _permission) {
        permission = _permission;
    }

    permissionType getPermission() {
        return permission;
    }

    ArduinoCloudPropertyGeneric& onUpdate(void(*fn)(void)) {
        callback = fn;
        return *(reinterpret_cast<ArduinoCloudPropertyGeneric*>(this));
    }

    void appendValue(CborObject &cbor);

    void append(CborObject &cbor) {
        if (tag != -1) {
            cbor.set("t", tag);
        } else {
            cbor.set("n", name.c_str());
        }
        appendValue(cbor);
        cbor.set("p", permission);
        lastUpdated = millis();
    }

    ArduinoCloudPropertyGeneric& publishEvery(long seconds) {
        updatePolicy = seconds;
        return *(reinterpret_cast<ArduinoCloudPropertyGeneric*>(this));
    }

    bool newData() {
        return (property != shadow_property);
    }

    bool shouldBeUpdated() {
        if (updatePolicy == ON_CHANGE) {
            return newData();
        }
        return (millis() - lastUpdated > updatePolicy * 1000) ;
    }

    inline bool operator==(const ArduinoCloudProperty& rhs){
        return (strcmp(getName(), rhs.getName) == 0);
    }

protected:
    T& property;
    T shadow_property;
    String name;
    int tag = -1;
    long lastUpdated = 0;
    long updatePolicy = ON_CHANGE;
    permissionType permission;
    static int tagIndex;
};

template <>
inline void ArduinoCloudProperty<int>::appendValue(CborObject &cbor) {
    cbor.set("i", property);
};

template <>
inline void ArduinoCloudProperty<bool>::appendValue(CborObject &cbor) {
    cbor.set("b", property);
};

template <>
inline void ArduinoCloudProperty<float>::appendValue(CborObject &cbor) {
    cbor.set("f", property);
};

template <>
inline void ArduinoCloudProperty<String>::appendValue(CborObject &cbor) {
    cbor.set("s", property.c_str());
};

template <>
inline void ArduinoCloudProperty<char*>::appendValue(CborObject &cbor) {
    cbor.set("s", property);
};

#ifndef addProperty
#define addProperty(prop, permission) addPropertyReal(prop, #prop, permission)
#endif

class ArduinoCloudThing {
public:
    ArduinoCloudThing();
    void begin(Client &client);
    ArduinoCloudPropertyGeneric& addPropertyReal(int& property, String name, permissionType permission);
    ArduinoCloudPropertyGeneric& addPropertyReal(bool& property, String name, permissionType permission);
    ArduinoCloudPropertyGeneric& addPropertyReal(float& property, String name, permissionType permission);
    ArduinoCloudPropertyGeneric& addPropertyReal(void* property, String name, permissionType permission);
    ArduinoCloudPropertyGeneric& addPropertyReal(String property, String name, permissionType permission);
    // poll should return > 0 if something has changed
    int poll();

private:
    static void callback(MQTTClient *client, char topic[], char bytes[], int length);
    bool connect();
    void publish(CborArray& object);

    void update();
    int checkNewData();
    void compress(CborArray& object, CborBuffer& buffer);
    void decode(uint8_t * payload, size_t length);

    bool exists(String &name);

    bool status = OFF;
    char uuid[33];

    LinkedList<ArduinoCloudPropertyGeneric*> list;
    int currentListIndex = -1;

    MQTTClient* client;
};

#endif
