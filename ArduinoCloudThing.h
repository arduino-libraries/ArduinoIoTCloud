#ifndef ArduinoCloudThing_h
#define ArduinoCloudThing_h

#include <Client.h>
#include <Stream.h>
#include "lib/mqtt/MQTTClient.h"
#include "lib/Network.h"
#include "lib/Timer.h"
#include <LinkedList.h>
#include <CborDecoder.h>
#include <CborEncoder.h>

#ifndef MQTT_BUFFER_SIZE
#define MQTT_BUFFER_SIZE 256
#endif

#define Serial DebugSerial
#define TESTING

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
    virtual const char* getName() = 0;
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

    T read() {
        if (permission != WRITE) {
            return property;
        }
    }

    const char* getName() {
        return name.c_str();
    }

    void appendValue(CborWriter &cbor);

    void append(CborWriter &cbor) {
        cbor.writeArray(3);
        appendValue(cbor);
        cbor.writeInt(permission);
        cbor.writeString(name);
    }

    inline bool operator==(const ArduinoCloudProperty& rhs){
        return (strcmp(getName(), rhs.getName) == 0);
    }

protected:
    T& property;
    String name;
    permissionType permission;
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
    // poll should return > 0 if something has changed
    int poll();

private:
    ArduinoCloudThing callback(MQTT::MessageData& messageData);
    bool connect();
    void publish();

    void update();
    void checkNewData(int* new_data_in, int* new_data_out);
    void decodeCBORData(uint8_t * payload, size_t length);

    bool status = OFF;
    char* topic;

    LinkedList<ArduinoCloudPropertyGeneric*> list;
    LinkedList<ArduinoCloudPropertyGeneric*> list_shadow;

    Network network;
    MQTT::Client<ArduinoCloudThing, Network, Timer, MQTT_BUFFER_SIZE, 0> * client;
    MQTTPacket_connectData options;
};

class CborPropertyListener : public CborListener {
  public:
    CborPropertyListener(LinkedList<ArduinoCloudPropertyGeneric*> _list) : list(_list) {}
    void OnInteger(int32_t value);
    void OnBytes(unsigned char *data, unsigned int size);
    void OnString(String &str);
    void OnArray(unsigned int size);
    void OnMap(unsigned int size) ;
    void OnTag(uint32_t tag);
    void OnSpecial(uint32_t code);
    void OnError(const char *error);
    LinkedList<ArduinoCloudPropertyGeneric*> list;
};

#endif
