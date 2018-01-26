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

class ArduinoCloudPropertyGeneric
{
public:
    virtual void append(CborObject& object) = 0;
    virtual String& getName() = 0;
    virtual void setName(String _name) = 0;
    virtual void setPermission(permissionType _permission) = 0;
    virtual permissionType getPermission() = 0;
    virtual bool newData() = 0;
    virtual void updateShadow() = 0;
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

    void setPermission(permissionType _permission) {
        permission = _permission;
    }

    permissionType getPermission() {
        return permission;
    }

    void appendValue(CborObject &cbor);

    void append(CborObject &cbor) {
        cbor.set("n", name.c_str());
        appendValue(cbor);
        cbor.set("p", permission);
    }

    bool newData() {
        return (property != shadow_property);
    }

    inline bool operator==(const ArduinoCloudProperty& rhs){
        return (strcmp(getName(), rhs.getName) == 0);
    }

protected:
    T& property;
    T shadow_property;
    String name;
    int tag;
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
    void addPropertyReal(int& property, String name, permissionType permission);
    void addPropertyReal(bool& property, String name, permissionType permission);
    void addPropertyReal(float& property, String name, permissionType permission);
    void addPropertyReal(void* property, String name, permissionType permission);
    void addPropertyReal(String property, String name, permissionType permission);
    // poll should return > 0 if something has changed
    int poll();

private:
    static void callback(MQTTClient *client, char topic[], char bytes[], int length);
    bool connect();
    void publish(CborObject& object);

    void update();
    int checkNewData();
    void compress(CborObject& object, CborBuffer& buffer);
    void decode(uint8_t * payload, size_t length);

    bool exists(String &name);

    bool status = OFF;
    char uuid[33];

    LinkedList<ArduinoCloudPropertyGeneric*> list;

    MQTTClient* client;
};

#endif
