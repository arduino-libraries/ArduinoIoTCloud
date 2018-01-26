#ifndef ArduinoCloudThing_h
#define ArduinoCloudThing_h

#include <Client.h>
#include <Stream.h>
#include "lib/MQTT/src/MQTTClient.h"
#include "lib/LinkedList/LinkedList.h"
#include "lib/CBOR/CborDecoder.h"
#include "lib/CBOR/CborEncoder.h"

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
    virtual void append(CborWriter &cbor) = 0;
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

    void appendValue(CborWriter &cbor);

    void append(CborWriter &cbor) {
        cbor.writeArray(4);
        cbor.writeTag(tag);
        cbor.writeString(name);
        appendValue(cbor);
        cbor.writeSpecial(permission);
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
inline void ArduinoCloudProperty<int>::appendValue(CborWriter &cbor) {
    cbor.writeInt(property);
};

template <>
inline void ArduinoCloudProperty<bool>::appendValue(CborWriter &cbor) {
    //cbor.writeBoolean(property);
    cbor.writeInt((int)property);
};

template <>
inline void ArduinoCloudProperty<float>::appendValue(CborWriter &cbor) {
    //cbor.writeFloat(property);
};

template <>
inline void ArduinoCloudProperty<String>::appendValue(CborWriter &cbor) {
    cbor.writeString(property);
};

template <>
inline void ArduinoCloudProperty<char*>::appendValue(CborWriter &cbor) {
    cbor.writeString(property);
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
    void publish(CborDynamicOutput& output);

    void update();
    int checkNewData(CborDynamicOutput& output);
    void compress(CborDynamicOutput& output, int howMany);
    void decode(uint8_t * payload, size_t length);

    bool exists(String &name);

    bool status = OFF;
    char uuid[33];

    LinkedList<ArduinoCloudPropertyGeneric*> list;

    MQTTClient* client;
};

class CborPropertyListener : public CborListener {
  public:
    CborPropertyListener(LinkedList<ArduinoCloudPropertyGeneric*> *_list) : list(_list) {}
    void OnInteger(int32_t value);
    void OnBytes(unsigned char *data, unsigned int size);
    void OnString(String &str);
    void OnArray(unsigned int size);
    void OnMap(unsigned int size) ;
    void OnTag(uint32_t tag);
    void OnSpecial(uint32_t code);
    void OnError(const char *error);
    LinkedList<ArduinoCloudPropertyGeneric*> *list;
    int currentListIndex;
    bool justStarted = true;
    int list_size = 0;
    bool newElement = false;
};

#endif
